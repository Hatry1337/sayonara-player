#ifndef HISTORYTABLEVIEW_H
#define HISTORYTABLEVIEW_H

#include <QTableView>

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Widgets/Dragable.h"

namespace Session
{
	class Manager;
}

class HistoryTableView :
	public Gui::WidgetTemplate<QTableView>,
	public Gui::Dragable
{
	Q_OBJECT
	PIMPL(HistoryTableView)

	signals:
		void sigRowcountChanged();

	public:
		explicit HistoryTableView(Session::Manager* sessionManager, Session::Timecode timecode, QWidget* parent=nullptr);
		~HistoryTableView() override;

		int rows() const;

	private slots:
		void rowcountChanged();

	protected:
		void skinChanged() override;

		void resizeEvent(QResizeEvent* e) override;
		void showEvent(QShowEvent* e) override;
};

#endif // HISTORYTABLEVIEW_H
