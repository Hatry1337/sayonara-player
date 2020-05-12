#include "HistoryTableView.h"
#include "HistoryEntryModel.h"

#include "Utils/Language/Language.h"

#include <QHeaderView>
#include <QStringListModel>
#include <QScrollBar>
#include <QDrag>

using Parent=Gui::WidgetTemplate<QTableView>;

struct HistoryTableView::Private
{
	HistoryEntryModel* model=nullptr;
};

HistoryTableView::HistoryTableView(Session::Timecode timecode, QWidget* parent) :
	Gui::WidgetTemplate<QTableView>(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>();

	m->model = new HistoryEntryModel(timecode, nullptr);
	this->setModel(m->model);

	this->setAlternatingRowColors(true);
	this->verticalHeader()->setVisible(false);
	this->horizontalHeader()->setStretchLastSection(true);
	this->setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

	skinChanged();

	connect(m->model, &HistoryEntryModel::sigRowsAdded, this, &HistoryTableView::rowcountChanged);
}

HistoryTableView::~HistoryTableView() = default;

int HistoryTableView::rows() const
{
	return model()->rowCount();
}

void HistoryTableView::rowcountChanged()
{
	skinChanged();

	emit sigRowcountChanged();
}

void HistoryTableView::languageChanged() {}

void HistoryTableView::skinChanged()
{
	int allHeight = (m->model->rowCount() * (this->fontMetrics().height() + 2)) +
		horizontalHeader()->height() * 2 +
		horizontalScrollBar()->height();

	this->setMinimumHeight(std::min(allHeight, 400));
}

QMimeData* HistoryTableView::dragableMimedata() const
{
	return m->model->mimeData(this->selectionModel()->selectedIndexes());
}

void HistoryTableView::resizeEvent(QResizeEvent* e)
{
	QTableView::resizeEvent(e);

	this->resizeColumnToContents(0);
//	int w = this->columnWidth(0);

//	this->setColumnWidth(1, (this->width() - w) / 3);
//	this->setColumnWidth(2, (this->width() - w) / 3);
//	this->setColumnWidth(3, (this->width() - w) / 3);
}
