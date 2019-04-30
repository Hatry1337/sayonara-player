#ifndef GUI_CONFIGURESTREAMS_H
#define GUI_CONFIGURESTREAMS_H

#include <QObject>
#include "GUI/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_ConfigureStreams)

class GUI_ConfigureStreams :
	public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS(GUI_ConfigureStreams)

public:

	enum Mode
	{New, Edit};

	GUI_ConfigureStreams(const QString& type, Mode mode, QWidget* parent=nullptr);
	~GUI_ConfigureStreams();

	QString url() const;
	QString name() const;

	void set_url(const QString& url);
	void set_name(const QString& name);
	void set_error_message(const QString& message);

	void set_mode(const QString& trype, Mode mode);
	bool was_accepted() const;
};

#endif // GUI_CONFIGURESTREAMS_H
