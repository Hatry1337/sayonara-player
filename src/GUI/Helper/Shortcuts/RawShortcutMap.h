#ifndef RAWSHORTCUTMAP_H
#define RAWSHORTCUTMAP_H


#include <QString>
#include <QMap>

/**
 * @brief The RawShortcutMap struct consisting of a specifier writable into database and a shortcut.
 * This class is used for converting a shortcut map into its database representation
 * @ingroup Shortcuts
 */
struct RawShortcutMap : public QMap<QString, QStringList> {

	QString toString() const;

	static RawShortcutMap fromString(const QString& setting);

};



#endif // RAWSHORTCUTMAP_H
