#ifndef GUI_ICONPREFERENCES_H
#define GUI_ICONPREFERENCES_H

#include "Interfaces/PreferenceDialog/PreferenceWidgetInterface.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_IconPreferences)

class QWidget;
class GUI_IconPreferences :
		public PreferenceWidgetInterface
{
	Q_OBJECT
	PIMPL(GUI_IconPreferences)
	UI_CLASS(GUI_IconPreferences)

public:
	explicit GUI_IconPreferences(QWidget* parent=nullptr);
	virtual ~GUI_IconPreferences();

protected:
	void init_ui() override;
	void retranslate_ui() override;

public:
	QString get_action_name() const override;
	void commit() override;
	void revert() override;

private slots:
	void theme_changed(const QString& theme);
};

#endif // GUI_ICONPREFERENCES_H