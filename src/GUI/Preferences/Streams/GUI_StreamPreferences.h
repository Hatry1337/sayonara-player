#ifndef GUI_STREAMPREFERENCES_H
#define GUI_STREAMPREFERENCES_H

#include "Interfaces/PreferenceDialog/PreferenceWidgetInterface.h"

UI_FWD(GUI_StreamPreferences)

class GUI_StreamPreferences :
	public PreferenceWidgetInterface
{
	Q_OBJECT
	UI_CLASS(GUI_StreamPreferences)

public:
	GUI_StreamPreferences(QWidget* parent=nullptr);
	~GUI_StreamPreferences();

	void commit() override;
	void revert() override;

	QString get_action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;
};

#endif // GUI_STREAMPREFERENCES_H