#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPlainTextEdit>
#include <QMessageBox>
#include <sstream>
#include "dfa2MiniDfa.h"
#include <QTableWidget>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lastNfa(nullptr, nullptr) // Initialize lastNfa with nullptrs
{
    ui->setupUi(this);
    connect(ui->BtnRegex2Postfix, &QPushButton::clicked, this, &MainWindow::on_BtnRegex2Postfix_clicked);
    connect(ui->BtnPostfix2Nfa, &QPushButton::clicked, this, &MainWindow::on_BtnPostfix2Nfa_clicked);
    connect(ui->BtnNfa2Dfa, &QPushButton::clicked, this, &MainWindow::on_BtnNfa2Dfa_clicked);
    connect(ui->BtnDfa2MiniDfa, &QPushButton::clicked, this, &MainWindow::on_BtnDfa2MiniDfa_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_BtnRegex2Postfix_clicked() {
    QString input = ui->TxtInput->toPlainText();
    std::string regex = input.toStdString();
    if (regex.empty()) {
        ui->TxtOutput->hide();
        ui->TblOutput->hide();
        QMessageBox::warning(this, "错误", "请输入正则表达式");
        return;
    }
    for (char c : regex) {
        if (!(isOperand(c) || c == '|' || c == '*' || c == '(' || c == ')')) {
            ui->TxtOutput->hide();
            ui->TblOutput->hide();
            QMessageBox::warning(this, "警告", "只支持字母(a-z, A-Z)、|、*、(、) 及其组合的正则表达式！");
            return;
        }
    }
    lastPostfix = regex2Postfix(regex);
    ui->TxtOutput->setPlainText(QString::fromStdString("Postfix: " + lastPostfix + "\n(仅支持字母、|、*、(、) 及其组合)"));
    ui->TxtOutput->show();
    ui->TblOutput->hide();

    // Clear all intermediate state
    lastNfaEdges.clear();
    lastDfaEdges.clear();
    lastMiniClasses.clear();
    lastMiniEdges.clear();
    lastNfaPtr = nullptr;
}

// Helper: fill NFA table
void MainWindow::fillNfaTable(const std::vector<Edge>& edges) {
    ui->TxtOutput->clear();
    ui->TblOutput->clear();
    ui->TblOutput->setRowCount(0);

    // Collect all states and all symbols (except epsilon)
    std::set<int> states;
    std::set<char> symbols;
    for (const auto& e : edges) {
        states.insert(e.begin->id);
        states.insert(e.end->id);
        if (e.label != 0) symbols.insert(e.label);
    }
    std::vector<int> stateVec(states.begin(), states.end());
    std::sort(stateVec.begin(), stateVec.end());
    std::vector<char> symbolVec(symbols.begin(), symbols.end());

    // Columns: State | symbol1 | ... | ε | Accept
    int colCount = 1 + (int)symbolVec.size() + 1 + 1; // State + symbols + ε + Accept
    ui->TblOutput->setColumnCount(colCount);

    QStringList headers;
    headers << "State";
    for (char c : symbolVec) headers << QString(c);
    headers << QString::fromUtf8("ε");
    headers << "Accept";
    ui->TblOutput->setHorizontalHeaderLabels(headers);

    int acceptId = lastNfa.Accept ? lastNfa.Accept->id : -1;
    int row = 0;
    for (int s : stateVec) {
        ui->TblOutput->insertRow(row);
        ui->TblOutput->setItem(row, 0, new QTableWidgetItem(QString::number(s)));

        // Symbol transitions
        for (size_t col = 0; col < symbolVec.size(); ++col) {
            std::set<int> toStates;
            for (const auto& e : edges) {
                if (e.begin->id == s && e.label == symbolVec[col])
                    toStates.insert(e.end->id);
            }
            QString cell;
            for (int t : toStates) cell += QString::number(t) + ",";
            if (!cell.isEmpty()) cell.chop(1);
            ui->TblOutput->setItem(row, col + 1, new QTableWidgetItem(cell));
        }

        // Epsilon transitions (label == 0)
        std::set<int> epsTo;
        for (const auto& e : edges) {
            if (e.begin->id == s && e.label == 0)
                epsTo.insert(e.end->id);
        }
        QString epsCell;
        for (int t : epsTo) epsCell += QString::number(t) + ",";
        if (!epsCell.isEmpty()) epsCell.chop(1);
        ui->TblOutput->setItem(row, (int)symbolVec.size() + 1, new QTableWidgetItem(epsCell));

        // Accept column
        QTableWidgetItem* acceptItem = new QTableWidgetItem((s == acceptId) ? "Yes" : "");
        if (s == acceptId) acceptItem->setBackground(Qt::yellow);
        ui->TblOutput->setItem(row, colCount - 1, acceptItem);

        ++row;
    }
    ui->TblOutput->resizeColumnsToContents();
}

// Helper: fill DFA table
void MainWindow::fillDfaTable(const std::map<DfaNode*, std::map<char, DfaNode*>>& dfaEdges) {
    ui->TxtOutput->clear();
    ui->TblOutput->clear();
    ui->TblOutput->setRowCount(0);
    ui->TblOutput->setColumnCount(0);
    std::set<int> states;
    std::set<char> symbols;
    std::map<int, bool> acceptMap;
    std::map<int, DfaNode*> idToNode;
    for (const auto& s : dfaEdges) {
        states.insert(s.first->id);
        acceptMap[s.first->id] = s.first->isAccept;
        idToNode[s.first->id] = s.first;
        for (const auto& t : s.second) {
            symbols.insert(t.first);
            states.insert(t.second->id);
            acceptMap[t.second->id] = t.second->isAccept;
            idToNode[t.second->id] = t.second;
        }
    }
    std::vector<int> stateVec(states.begin(), states.end());
    std::sort(stateVec.begin(), stateVec.end());
    std::vector<char> symbolVec(symbols.begin(), symbols.end());
    int colCount = (int)symbolVec.size() + 2;
    ui->TblOutput->setColumnCount(colCount);
    QStringList headers;
    headers << "State";
    for (char c : symbolVec) headers << QString(c);
    headers << "Accept";
    ui->TblOutput->setHorizontalHeaderLabels(headers);
    int row = 0;
    for (int s : stateVec) {
        // Ignore dead state (nfaStates is empty)
        if (idToNode[s]->nfaStates.empty()) continue;
        ui->TblOutput->insertRow(row);
        ui->TblOutput->setItem(row, 0, new QTableWidgetItem(QString::number(s)));
        for (size_t col = 0; col < symbolVec.size(); ++col) {
            QString cell;
            for (const auto& from : dfaEdges) {
                if (from.first->id == s && from.second.count(symbolVec[col])) {
                    // Also ignore dead state as target
                    if (from.second.at(symbolVec[col])->nfaStates.empty()) break;
                    cell = QString::number(from.second.at(symbolVec[col])->id);
                    break;
                }
            }
            ui->TblOutput->setItem(row, col + 1, new QTableWidgetItem(cell));
        }
        QTableWidgetItem* acceptItem = new QTableWidgetItem(acceptMap[s] ? "Yes" : "");
        if (acceptMap[s]) acceptItem->setBackground(Qt::yellow);
        ui->TblOutput->setItem(row, colCount - 1, acceptItem);
        ++row;
    }
    ui->TblOutput->resizeColumnsToContents();
}

// Helper: fill MiniDFA table
void MainWindow::fillMiniDfaTable(const std::vector<MiniEdge*>& miniEdges) {
    ui->TxtOutput->clear();
    ui->TblOutput->clear();
    ui->TblOutput->setRowCount(0);
    ui->TblOutput->setColumnCount(0);
    std::set<int> states;
    std::set<char> symbols;
    std::map<int, bool> acceptMap;
    for (const auto& e : miniEdges) {
        states.insert(e->begin->id);
        states.insert(e->end->id);
        symbols.insert(e->label);
        // Accept if any dfaState in the set is accept
        for (auto dfaNode : e->begin->dfaStates) {
            if (dfaNode->isAccept) acceptMap[e->begin->id] = true;
        }
        for (auto dfaNode : e->end->dfaStates) {
            if (dfaNode->isAccept) acceptMap[e->end->id] = true;
        }
    }
    std::vector<int> stateVec(states.begin(), states.end());
    std::sort(stateVec.begin(), stateVec.end());
    std::vector<char> symbolVec(symbols.begin(), symbols.end());
    int colCount = (int)symbolVec.size() + 2;
    ui->TblOutput->setColumnCount(colCount);
    QStringList headers;
    headers << "MiniNode";
    for (char c : symbolVec) headers << QString(c);
    headers << "Accept";
    ui->TblOutput->setHorizontalHeaderLabels(headers);
    int row = 0;
    for (int s : stateVec) {
        ui->TblOutput->insertRow(row);
        ui->TblOutput->setItem(row, 0, new QTableWidgetItem(QString::number(s)));
        for (size_t col = 0; col < symbolVec.size(); ++col) {
            QString cell;
            for (const auto& e : miniEdges) {
                if (e->begin->id == s && e->label == symbolVec[col]) {
                    cell = QString::number(e->end->id);
                    break;
                }
            }
            ui->TblOutput->setItem(row, col + 1, new QTableWidgetItem(cell));
        }
        QTableWidgetItem* acceptItem = new QTableWidgetItem(acceptMap[s] ? "Yes" : "");
        if (acceptMap[s]) acceptItem->setBackground(Qt::yellow);
        ui->TblOutput->setItem(row, colCount - 1, acceptItem);
        ++row;
    }
    ui->TblOutput->resizeColumnsToContents();
}

void MainWindow::on_BtnPostfix2Nfa_clicked() {
    if (lastPostfix.empty()) {
        ui->TxtOutput->hide();
        ui->TblOutput->hide();
        QMessageBox::warning(this, "错误", "请先生成后缀表达式");
        return;
    }
    lastNfaEdges.clear();
    lastNfa = postfix2Nfa(lastPostfix, lastNfaEdges);
    reassignNodeIds(lastNfa, lastNfaEdges); 
    lastNfaPtr = &lastNfa;
    fillNfaTable(lastNfaEdges);
    ui->TblOutput->show();
    ui->TxtOutput->hide();
}

void MainWindow::on_BtnNfa2Dfa_clicked() {
    if (!lastNfaPtr) {
        ui->TxtOutput->hide();
        ui->TblOutput->hide();
        QMessageBox::warning(this, "错误", "请先生成NFA");
        return;
    }
    lastDfaEdges = nfa2Dfa(lastNfaPtr, lastNfaEdges);
    fillDfaTable(lastDfaEdges);
    ui->TblOutput->show();
    ui->TxtOutput->hide();
}

void MainWindow::on_BtnDfa2MiniDfa_clicked() {
    if (lastDfaEdges.empty()) {
        ui->TxtOutput->hide();
        ui->TblOutput->hide();
        QMessageBox::warning(this, "错误", "请先生成DFA");
        return;
    }
    std::map<std::pair<int, int>, bool> distinguishable;
    initDisTable(lastDfaEdges, distinguishable);
    splitAndMark(lastDfaEdges, distinguishable);
    markDisPairs(lastDfaEdges, distinguishable);
    lastMiniClasses = equivalenceClasses(distinguishable, lastDfaEdges);
    lastMiniEdges = buildMiniDfa(lastDfaEdges, lastMiniClasses);
    fillMiniDfaTable(lastMiniEdges);
    ui->TblOutput->show();
    ui->TxtOutput->hide();
}
