#include "LineInputDialog.h"
#include <QInputDialog>
#include "Utils/Language.h"

LineInputDialog::LineInputDialog(const QString& title, const QString& label, QWidget* parent) :
	QInputDialog(parent)
{
	setWindowTitle(title);
	setLabelText(label);
	setModal(true);
	setCancelButtonText(Lang::get(Lang::Cancel));
	setOkButtonText(Lang::get(Lang::OK));
	setInputMode(QInputDialog::TextInput);
}

LineInputDialog::LineInputDialog(const QString& title, const QString& label, const QString& initial_value, QWidget* parent) :
	LineInputDialog(title, label, parent)
{
	setTextValue(initial_value);
}

LineInputDialog::~LineInputDialog() {}
