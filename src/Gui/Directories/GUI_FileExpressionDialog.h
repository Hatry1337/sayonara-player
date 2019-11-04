#ifndef GUI_FILEEXPRESSIONDIALOG_H
#define GUI_FILEEXPRESSIONDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class GUI_FileExpressionDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_FileExpressionDialog)

public:
	GUI_FileExpressionDialog(QWidget* parent=nullptr);
	~GUI_FileExpressionDialog() override;

	QString expression() const;

protected:
	void showEvent(QShowEvent* event) override;
	void language_changed() override;

private slots:
	void btn_clicked();
	void text_changed(const QString& text);
};

#endif // GUI_FILEEXPRESSIONDIALOG_H
