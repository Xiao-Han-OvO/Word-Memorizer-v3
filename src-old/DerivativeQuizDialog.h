#ifndef DERIVATIVEQUIZDIALOG_H
#define DERIVATIVEQUIZDIALOG_H

#include <gtkmm.h>
#include "WordManager.h"

class DerivativeQuizDialog : public Gtk::Dialog {
private:
    // 当前检测状态
    std::vector<Word> derivatives;
    size_t currentDerivativeIndex;
    int correctCount;
    int totalCount;
    
    // UI组件
    Gtk::Box mainBox;
    Gtk::Box titleBox;
    Gtk::Box contentBox;
    Gtk::Box inputBox;
    Gtk::Box buttonBox;
    
    Gtk::Label titleLabel;
    Gtk::Label progressLabel;
    Gtk::Label posLabel;
    Gtk::Label meaningLabel;
    Gtk::Label exampleLabel;
    Gtk::Label feedbackLabel;
    
    Gtk::Entry answerEntry;
    Gtk::Button submitButton;
    Gtk::Button skipButton;
    Gtk::Button closeButton;
    
    Gtk::ProgressBar progressBar;

public:
    DerivativeQuizDialog(Gtk::Window& parent, const std::vector<Word>& derivatives);
    virtual ~DerivativeQuizDialog();
    
    // 获取测试结果
    int getCorrectCount() const { return correctCount; }
    int getTotalCount() const { return totalCount; }
    float getAccuracy() const { return totalCount > 0 ? (float)correctCount / totalCount : 0.0f; }

private:
    void initializeUI();
    void showCurrentDerivative();
    void on_submit_clicked();
    void on_skip_clicked();
    void on_close_clicked();
    void on_entry_activate();
    void showResults();
    void updateProgress();
};

#endif