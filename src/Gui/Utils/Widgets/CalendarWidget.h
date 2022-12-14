#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include <QCalendarWidget>

namespace Gui
{
	class CalendarWidget : public QCalendarWidget
	{
		public:
			CalendarWidget(QWidget* parent=nullptr);
			~CalendarWidget() override;

		protected:
			void paintCell(QPainter* painter, const QRect& rect, const QDate& date) const override;
	};
}

#endif // CALENDARWIDGET_H
