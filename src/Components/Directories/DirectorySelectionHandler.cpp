#include "DirectorySelectionHandler.h"
#include "MetaDataScanner.h"
#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Algorithm.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Filter.h"



#include <QThread>
#include <QList>

struct DirectorySelectionHandler::Private
{
public:
	LocalLibrary*			generic_library=nullptr;

	int						current_library_index;
	QList<Library::Info>	libraries;

	Private()
	{
		libraries = Library::Manager::instance()->all_libraries();
		current_library_index = (libraries.count() > 0) ? 0 : -1;
	}
};

DirectorySelectionHandler::DirectorySelectionHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto* library_manager = Library::Manager::instance();
	connect(library_manager, &Library::Manager::sig_added, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		libraries_changed();
	});

	connect(library_manager, &Library::Manager::sig_removed, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		libraries_changed();
	});

	connect(library_manager, &Library::Manager::sig_moved, this, [this](auto i1, auto i2, auto i3)
	{
		Q_UNUSED(i1)
		Q_UNUSED(i2)
		Q_UNUSED(i3)
		libraries_changed();
	});

	connect(library_manager, &Library::Manager::sig_renamed, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		libraries_changed();
	});
}

DirectorySelectionHandler::~DirectorySelectionHandler() = default;

void DirectorySelectionHandler::create_playlist(const QStringList& paths, bool create_new_playlist)
{
	auto* plh = Playlist::Handler::instance();

	if(create_new_playlist) {
		plh->create_playlist(paths, plh->request_new_playlist_name());
	}

	else {
		plh->create_playlist(paths);
	}
}

void DirectorySelectionHandler::play_next(const QStringList& paths)
{
	auto* plh = Playlist::Handler::instance();
	plh->play_next(paths);
}

void DirectorySelectionHandler::append_tracks(const QStringList& paths)
{
	auto* plh = Playlist::Handler::instance();
	plh->append_tracks(paths, plh->current_index());
}

void DirectorySelectionHandler::prepare_tracks_for_playlist(const QStringList& paths, bool create_new_playlist)
{
	this->library_instance()->prepare_tracks_for_playlist(paths, create_new_playlist);
}

void DirectorySelectionHandler::import_requested(LibraryId lib_id, const QStringList& paths, const QString& target_dir)
{
	if(lib_id != this->library_id() || lib_id < 0){
		return;
	}

	LocalLibrary* library = this->library_instance();
	connect(library, &LocalLibrary::sig_import_dialog_requested,
			this, &DirectorySelectionHandler::sig_import_dialog_requested);

	// prepare import
	library->import_files_to(paths, target_dir);
}

FileOperations* DirectorySelectionHandler::create_file_operation()
{
	auto* fo = new FileOperations(this);

	connect(fo, &FileOperations::sig_started, this, &DirectorySelectionHandler::sig_file_operation_started);
	connect(fo, &FileOperations::sig_finished, this, &DirectorySelectionHandler::sig_file_operation_finished);
	connect(fo, &FileOperations::sig_finished, fo, &QObject::deleteLater);

	return fo;
}

void DirectorySelectionHandler::copy_paths(const QStringList& paths, const QString& target_dir)
{
	create_file_operation()->copy_paths(paths, target_dir);
}

void DirectorySelectionHandler::move_paths(const QStringList& paths, const QString& target_dir)
{
	create_file_operation()->move_paths(paths, target_dir);
}

void DirectorySelectionHandler::rename_path(const QString& path, const QString& new_name)
{
	create_file_operation()->rename_path(path, new_name);
}

void DirectorySelectionHandler::rename_by_expression(const QString& path, const QString& expression)
{
	create_file_operation()->rename_by_expression(path, expression);
}

void DirectorySelectionHandler::delete_paths(const QStringList& paths)
{
	create_file_operation()->delete_paths(paths);
}

void DirectorySelectionHandler::libraries_changed()
{
	LibraryId id = library_id();
	m->libraries = Library::Manager::instance()->all_libraries();

	int index = Util::Algorithm::indexOf(m->libraries, [&id](const Library::Info& info){
		return (info.id() == id);
	});

	m->current_library_index = index;

	emit sig_libraries_changed();
}

void DirectorySelectionHandler::set_library_id(LibraryId lib_id)
{
	m->current_library_index = Util::Algorithm::indexOf(m->libraries, [&lib_id](const Library::Info& info){
		return (info.id() == lib_id);
	});
}

LibraryId DirectorySelectionHandler::library_id() const
{
	return library_info().id();
}

void DirectorySelectionHandler::create_new_library(const QString& name, const QString& path)
{
	Library::Manager::instance()->add_library(name, path);
}

Library::Info DirectorySelectionHandler::library_info() const
{
	if(!Util::between(m->current_library_index, m->libraries))
	{
		return Library::Info();
	}

	return m->libraries[m->current_library_index];
}

LocalLibrary* DirectorySelectionHandler::library_instance() const
{
	auto* manager = Library::Manager::instance();
	LibraryId lib_id = library_info().id();
	auto* library = manager->library_instance(lib_id);

	if(library == nullptr)
	{
		if(!m->generic_library){
			m->generic_library = Library::Manager::instance()->library_instance(-1);
		}

		sp_log(Log::Warning, this) << "Invalid library index";
		return m->generic_library;
	}

	return library;
}

void DirectorySelectionHandler::set_search_text(const QString& text)
{
	Library::Filter filter;
	filter.set_filtertext(text, GetSetting(Set::Lib_SearchMode));
	filter.set_mode(Library::Filter::Mode::Filename);

	library_instance()->change_filter(filter);
}
