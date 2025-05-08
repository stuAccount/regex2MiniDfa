#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <vector>
#include <map>
#include <set>
#include "regex2Postfix.h"
#include "postfix2Nfa.h"
#include "nfa2Dfa.h"
#include "dfa2MiniDfa.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_BtnRegex2Postfix_clicked();
    void on_BtnPostfix2Nfa_clicked();
    void on_BtnNfa2Dfa_clicked();
    void on_BtnDfa2MiniDfa_clicked();

private:
    Ui::MainWindow *ui;
    std::string lastPostfix;
    Fragment lastNfa;
    std::vector<Edge> lastNfaEdges;
    std::map<DfaNode*, std::map<char, DfaNode*>> lastDfaEdges;
    std::vector<std::set<DfaNode*>> lastMiniClasses;
    std::vector<MiniEdge*> lastMiniEdges;
    Fragment* lastNfaPtr = nullptr;

    void fillNfaTable(const std::vector<Edge>& edges);
    void fillDfaTable(const std::map<DfaNode*, std::map<char, DfaNode*>>& dfaEdges);
    void fillMiniDfaTable(const std::vector<MiniEdge*>& miniEdges);
};
#endif // MAINWINDOW_H
