/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qitemmodelbardataproxy_p.h"
#include "baritemmodelhandler_p.h"

QT_BEGIN_NAMESPACE_DATAVISUALIZATION

/*!
 * \class QItemModelBarDataProxy
 * \inmodule QtDataVisualization
 * \brief Proxy class for presenting data in item models with Q3DBars.
 * \since QtDataVisualization 1.0
 *
 * QItemModelBarDataProxy allows you to use QAbstractItemModel derived models as a data source
 * for Q3DBars. It uses the defined mappings to map data from the model to rows, columns, and
 * values of Q3DBars graph.
 *
 * The data is resolved asynchronously whenever mappings or the model changes.
 * QBarDataProxy::arrayReset() is emitted when the data has been resolved.
 * However, when useModelCategories property is set to true, single item changes are resolved
 * synchronously, unless the same frame also contains a change that causes the whole model to be
 * resolved.
 *
 * Mappings can be used in the following ways:
 *
 * \list
 * \li If useModelCategories property is set to true, this proxy will map rows and
 *    columns of QAbstractItemModel directly to rows and columns of Q3DBars, and uses the value
 *    returned for Qt::DisplayRole as bar value by default.
 *    The value role to be used can be redefined if Qt::DisplayRole is not suitable.
 *
 * \li For models that do not have data already neatly sorted into rows and columns, such as
 *    QAbstractListModel based models, you can define a role from the model to map for each of row,
 *    column and value.
 *
 * \li If you do not want to include all data contained in the model, or the autogenerated rows and
 *    columns are not ordered as you wish, you can specify which rows and columns should be included
 *    and in which order by defining an explicit list of categories for either or both of rows and
 *    columns.
 * \endlist
 *
 * For example, assume that you have a custom QAbstractItemModel for storing various monthly values
 * related to a business.
 * Each item in the model has the roles "year", "month", "income", and "expenses".
 * You could do the following to display the data in a bar graph:
 *
 * \snippet doc_src_qtdatavisualization.cpp 3
 *
 * If the fields of the model do not contain the data in the exact format you need, you can specify
 * a search pattern regular expression and a replace rule for each role to get the value in a
 * format you need. For more information how the replace using regular expressions works, see
 * QString::replace(const QRegExp &rx, const QString &after) function documentation. Note that
 * using regular expressions has an impact on the performance, so it's more efficient to utilize
 * item models where doing search and replace is not necessary to get the desired values.
 *
 * For example about using the search patterns in conjunction with the roles, see
 * \l{Qt Quick 2 Bars Example}.
 *
 * \sa {Qt Data Visualization Data Handling}
 */

/*!
 * \qmltype ItemModelBarDataProxy
 * \inqmlmodule QtDataVisualization
 * \since QtDataVisualization 1.0
 * \ingroup datavisualization_qml
 * \instantiates QItemModelBarDataProxy
 * \inherits BarDataProxy
 * \brief Proxy class for presenting data in item models with Bars3D.
 *
 * This type allows you to use AbstractItemModel derived models as a data source for Bars3D.
 *
 * Data is resolved asynchronously whenever the mapping or the model changes.
 * QBarDataProxy::arrayReset() is emitted when the data has been resolved.
 *
 * For ItemModelBarDataProxy enums, see \l{QItemModelBarDataProxy::MultiMatchBehavior}.
 *
 * For more details, see QItemModelBarDataProxy documentation.
 *
 * Usage example:
 *
 * \snippet doc_src_qmldatavisualization.cpp 7
 *
 * \sa BarDataProxy, {Qt Data Visualization Data Handling}
 */

/*!
 * \qmlproperty model ItemModelBarDataProxy::itemModel
 * The item model.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rowRole
 * The item model role to map into row category.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::columnRole
 * The item model role to map into column category.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::valueRole
 * The item model role to map into bar value.
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rotationRole
 * The item model role to map into bar rotation angle.
 */

/*!
 * \qmlproperty list<String> ItemModelBarDataProxy::rowCategories
 * The row categories of the mapping. Only items with row role values that are found in this list
 * are included when the data is resolved. The rows are ordered in the same order as they are in
 * this list.
 */

/*!
 * \qmlproperty list<String> ItemModelBarDataProxy::columnCategories
 * The column categories of the mapping. Only items with column role values that are found in this
 * list are included when the data is resolved. The columns are ordered in the same order as they
 * are in this list.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::useModelCategories
 * When set to \c true, the mapping ignores row and column roles and categories, and uses
 * the rows and columns from the model instead. Row and column headers are used for row and column
 * labels. Defaults to \c{false}.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::autoRowCategories
 * When set to \c true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from the model is resolved. Defaults to \c{true}.
 */

/*!
 * \qmlproperty bool ItemModelBarDataProxy::autoColumnCategories
 * When set to \c true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::rowRolePattern
 * When set, a search and replace is done on the value mapped by row role before it is used as
 * a row category. This property specifies the regular expression to find the portion of the
 * mapped value to replace and rowRoleReplace property contains the replacement string.
 * This is useful for example in parsing row and column categories from a single
 * timestamp field in the item model.
 *
 * \sa rowRole, rowRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::columnRolePattern
 * When set, a search and replace is done on the value mapped by column role before it is used
 * as a column category. This property specifies the regular expression to find the portion of the
 * mapped value to replace and columnRoleReplace property contains the replacement string.
 * This is useful for example in parsing row and column categories from
 * a single timestamp field in the item model.
 *
 * \sa columnRole, columnRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::valueRolePattern
 * When set, a search and replace is done on the value mapped by value role before it is used as
 * a bar value. This property specifies the regular expression to find the portion of the
 * mapped value to replace and valueRoleReplace property contains the replacement string.
 *
 * \sa valueRole, valueRoleReplace
 */

/*!
 * \qmlproperty regExp ItemModelBarDataProxy::rotationRolePattern
 * When set, a search and replace is done on the value mapped by rotation role before it is used
 * as a bar rotation angle. This property specifies the regular expression to find the portion
 * of the mapped value to replace and rotationRoleReplace property contains the replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rowRoleReplace
 * This property defines the replace content to be used in conjunction with rowRolePattern.
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa rowRole, rowRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::columnRoleReplace
 * This property defines the replace content to be used in conjunction with columnRolePattern.
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa columnRole, columnRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::valueRoleReplace
 * This property defines the replace content to be used in conjunction with valueRolePattern.
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa valueRole, valueRolePattern
 */

/*!
 * \qmlproperty string ItemModelBarDataProxy::rotationRoleReplace
 * This property defines the replace content to be used in conjunction with rotationRolePattern.
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */

/*!
 * \qmlproperty ItemModelBarDataProxy.MultiMatchBehavior ItemModelBarDataProxy::multiMatchBehavior
 * Defines how multiple matches for each row/column combination are handled.
 * Defaults to \l{QItemModelBarDataProxy::MMBLast}{ItemModelBarDataProxy.MMBLast}. The chosen
 * behavior affects both bar value and rotation.
 *
 * For example, you might have an item model with timestamped data taken at irregular intervals
 * and you want to visualize total value of data items on each day with a bar graph.
 * This can be done by specifying row and column categories so that each bar represents a day,
 * and setting multiMatchBehavior to
 * \l{QItemModelBarDataProxy::MMBCumulative}{ItemModelBarDataProxy.MMBCumulative}.
 */

/*!
 *  \enum QItemModelBarDataProxy::MultiMatchBehavior
 *
 *  Behavior types for QItemModelBarDataProxy::multiMatchBehavior property.
 *
 *  \value MMBFirst
 *         The value is taken from the first item in the item model that matches
 *         each row/column combination.
 *  \value MMBLast
 *         The value is taken from the last item in the item model that matches
 *         each row/column combination.
 *  \value MMBAverage
 *         The values from all items matching each row/column combination are
 *         averaged together and the average is used as the bar value.
 *  \value MMBCumulative
 *         The values from all items matching each row/column combination are
 *         added together and the total is used as the bar value.
 */

/*!
 * Constructs QItemModelBarDataProxy with optional \a parent.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel, QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    setItemModel(itemModel);
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 * The value role is set to \a valueRole.
 * This constructor is meant to be used with models that have data properly sorted
 * in rows and columns already, so it also sets useModelCategories property to true.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &valueRole, QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
    dptr()->m_valueRole = valueRole;
    dptr()->m_useModelCategories = true;
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 * The role mappings are set with \a rowRole, \a columnRole, and \a valueRole.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole, QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 * The role mappings are set with \a rowRole, \a columnRole, \a valueRole, and \a rotationRole.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QString &rotationRole,
                                               QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->m_rotationRole = rotationRole;
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 * The role mappings are set with \a rowRole, \a columnRole, and \a valueRole.
 * Row and column categories are set with \a rowCategories and \a columnCategories.
 * This constructor also sets autoRowCategories and autoColumnCategories to false.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QStringList &rowCategories,
                                               const QStringList &columnCategories,
                                               QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->m_rowCategories = rowCategories;
    dptr()->m_columnCategories = columnCategories;
    dptr()->m_autoRowCategories = false;
    dptr()->m_autoColumnCategories = false;
    dptr()->connectItemModelHandler();
}

/*!
 * Constructs QItemModelBarDataProxy with \a itemModel and optional \a parent. Proxy doesn't take
 * ownership of the \a itemModel, as typically item models are owned by other controls.
 * The role mappings are set with \a rowRole, \a columnRole, \a valueRole, and \a rotationRole.
 * Row and column categories are set with \a rowCategories and \a columnCategories.
 * This constructor also sets autoRowCategories and autoColumnCategories to false.
 */
QItemModelBarDataProxy::QItemModelBarDataProxy(QAbstractItemModel *itemModel,
                                               const QString &rowRole,
                                               const QString &columnRole,
                                               const QString &valueRole,
                                               const QString &rotationRole,
                                               const QStringList &rowCategories,
                                               const QStringList &columnCategories,
                                               QObject *parent)
    : QBarDataProxy(new QItemModelBarDataProxyPrivate(this), parent)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->m_rotationRole = rotationRole;
    dptr()->m_rowCategories = rowCategories;
    dptr()->m_columnCategories = columnCategories;
    dptr()->m_autoRowCategories = false;
    dptr()->m_autoColumnCategories = false;
    dptr()->connectItemModelHandler();
}

/*!
 * Destroys QItemModelBarDataProxy.
 */
QItemModelBarDataProxy::~QItemModelBarDataProxy()
{
}

/*!
 * \property QItemModelBarDataProxy::itemModel
 *
 * \brief The item model.
 */

/*!
 * Sets the item model to \a itemModel. Does not take ownership of the model,
 * but does connect to it to listen for changes.
 */
void QItemModelBarDataProxy::setItemModel(QAbstractItemModel *itemModel)
{
    dptr()->m_itemModelHandler->setItemModel(itemModel);
}

QAbstractItemModel *QItemModelBarDataProxy::itemModel() const
{
    return dptrc()->m_itemModelHandler->itemModel();
}

/*!
 * \property QItemModelBarDataProxy::rowRole
 *
 * \brief The row role for the mapping.
 */
void QItemModelBarDataProxy::setRowRole(const QString &role)
{
    if (dptr()->m_rowRole != role) {
        dptr()->m_rowRole = role;
        emit rowRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::rowRole() const
{
    return dptrc()->m_rowRole;
}

/*!
 * \property QItemModelBarDataProxy::columnRole
 *
 * \brief The column role for the mapping.
 */
void QItemModelBarDataProxy::setColumnRole(const QString &role)
{
    if (dptr()->m_columnRole != role) {
        dptr()->m_columnRole = role;
        emit columnRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::columnRole() const
{
    return dptrc()->m_columnRole;
}

/*!
 * \property QItemModelBarDataProxy::valueRole
 *
 * \brief The value role for the mapping.
 */
void QItemModelBarDataProxy::setValueRole(const QString &role)
{
    if (dptr()->m_valueRole != role) {
        dptr()->m_valueRole = role;
        emit valueRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::valueRole() const
{
    return dptrc()->m_valueRole;
}

/*!
 * \property QItemModelBarDataProxy::rotationRole
 *
 * \brief The rotation role for the mapping.
 */
void QItemModelBarDataProxy::setRotationRole(const QString &role)
{
    if (dptr()->m_rotationRole != role) {
        dptr()->m_rotationRole = role;
        emit rotationRoleChanged(role);
    }
}

QString QItemModelBarDataProxy::rotationRole() const
{
    return dptrc()->m_rotationRole;
}

/*!
 * \property QItemModelBarDataProxy::rowCategories
 *
 * \brief The row categories for the mapping.
 */
void QItemModelBarDataProxy::setRowCategories(const QStringList &categories)
{
    if (dptr()->m_rowCategories != categories) {
        dptr()->m_rowCategories = categories;
        emit rowCategoriesChanged();
    }
}

QStringList QItemModelBarDataProxy::rowCategories() const
{
    return dptrc()->m_rowCategories;
}

/*!
 * \property QItemModelBarDataProxy::columnCategories
 *
 * \brief The column categories for the mapping.
 */
void QItemModelBarDataProxy::setColumnCategories(const QStringList &categories)
{
    if (dptr()->m_columnCategories != categories) {
        dptr()->m_columnCategories = categories;
        emit columnCategoriesChanged();
    }
}

QStringList QItemModelBarDataProxy::columnCategories() const
{
    return dptrc()->m_columnCategories;
}

/*!
 * \property QItemModelBarDataProxy::useModelCategories
 *
 * \brief Whether row and column roles and categories are used for mapping.
 *
 * When set to \c true, the mapping ignores row and column roles and categories, and uses
 * the rows and columns from the model instead. Defaults to \c{false}.
 */
void QItemModelBarDataProxy::setUseModelCategories(bool enable)
{
    if (dptr()->m_useModelCategories != enable) {
        dptr()->m_useModelCategories = enable;
        emit useModelCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::useModelCategories() const
{
    return dptrc()->m_useModelCategories;
}

/*!
 * \property QItemModelBarDataProxy::autoRowCategories
 *
 * \brief Whether row categories are generated automatically.
 *
 * When set to \c true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */
void QItemModelBarDataProxy::setAutoRowCategories(bool enable)
{
    if (dptr()->m_autoRowCategories != enable) {
        dptr()->m_autoRowCategories = enable;
        emit autoRowCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::autoRowCategories() const
{
    return dptrc()->m_autoRowCategories;
}

/*!
 * \property QItemModelBarDataProxy::autoColumnCategories
 *
 * \brief Whether column categories are generated automatically.
 *
 * When set to \c true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to \c{true}.
 */
void QItemModelBarDataProxy::setAutoColumnCategories(bool enable)
{
    if (dptr()->m_autoColumnCategories != enable) {
        dptr()->m_autoColumnCategories = enable;
        emit autoColumnCategoriesChanged(enable);
    }
}

bool QItemModelBarDataProxy::autoColumnCategories() const
{
    return dptrc()->m_autoColumnCategories;
}

/*!
 * Changes \a rowRole, \a columnRole, \a valueRole, \a rotationRole,
 * \a rowCategories and \a columnCategories to the mapping.
 */
void QItemModelBarDataProxy::remap(const QString &rowRole,
                                   const QString &columnRole,
                                   const QString &valueRole,
                                   const QString &rotationRole,
                                   const QStringList &rowCategories,
                                   const QStringList &columnCategories)
{
    setRowRole(rowRole);
    setColumnRole(columnRole);
    setValueRole(valueRole);
    setRotationRole(rotationRole);
    setRowCategories(rowCategories);
    setColumnCategories(columnCategories);
}

/*!
 * Returns the index of the specified \a category in row categories list.
 * If the row categories list is empty, -1 is returned.
 * \note If the automatic row categories generation is in use, this method will
 * not return a valid index before the data in the model is resolved for the first time.
 */
int QItemModelBarDataProxy::rowCategoryIndex(const QString &category)
{
    return dptr()->m_rowCategories.indexOf(category);
}

/*!
 * Returns the index of the specified \a category in column categories list.
 * If the category is not found, -1 is returned.
 * \note If the automatic column categories generation is in use, this method will
 * not return a valid index before the data in the model is resolved for the first time.
 */
int QItemModelBarDataProxy::columnCategoryIndex(const QString &category)
{
    return dptr()->m_columnCategories.indexOf(category);
}

/*!
 * \property QItemModelBarDataProxy::rowRolePattern
 *
 * \brief Whether a search and replace is performed on the value mapped by row
 *  role before it is used as a row category.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and rowRoleReplace property contains the replacement string.
 * This is useful for example in parsing row and column categories from a single
 * timestamp field in the item model.
 *
 * \sa rowRole, rowRoleReplace
 */
void QItemModelBarDataProxy::setRowRolePattern(const QRegExp &pattern)
{
    if (dptr()->m_rowRolePattern != pattern) {
        dptr()->m_rowRolePattern = pattern;
        emit rowRolePatternChanged(pattern);
    }
}

QRegExp QItemModelBarDataProxy::rowRolePattern() const
{
    return dptrc()->m_rowRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::columnRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by column
 * role before it is used as a column category.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and columnRoleReplace property contains the replacement string.
 * This is useful for example in parsing row and column categories from
 * a single timestamp field in the item model.
 *
 * \sa columnRole, columnRoleReplace
 */
void QItemModelBarDataProxy::setColumnRolePattern(const QRegExp &pattern)
{
    if (dptr()->m_columnRolePattern != pattern) {
        dptr()->m_columnRolePattern = pattern;
        emit columnRolePatternChanged(pattern);
    }
}

QRegExp QItemModelBarDataProxy::columnRolePattern() const
{
    return dptrc()->m_columnRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::valueRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by value role
 * before it is used as a bar value.
 *
 * This property specifies the regular expression to find the portion of the
 * mapped value to replace and valueRoleReplace property contains the replacement string.
 *
 * \sa valueRole, valueRoleReplace
 */
void QItemModelBarDataProxy::setValueRolePattern(const QRegExp &pattern)
{
    if (dptr()->m_valueRolePattern != pattern) {
        dptr()->m_valueRolePattern = pattern;
        emit valueRolePatternChanged(pattern);
    }
}

QRegExp QItemModelBarDataProxy::valueRolePattern() const
{
    return dptrc()->m_valueRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::rotationRolePattern
 *
 * \brief Whether a search and replace is done on the value mapped by rotation
 * role before it is used as a bar rotation angle.
 *
 * This property specifies the regular expression to find the portion
 * of the mapped value to replace and rotationRoleReplace property contains the replacement string.
 *
 * \sa rotationRole, rotationRoleReplace
 */
void QItemModelBarDataProxy::setRotationRolePattern(const QRegExp &pattern)
{
    if (dptr()->m_rotationRolePattern != pattern) {
        dptr()->m_rotationRolePattern = pattern;
        emit rotationRolePatternChanged(pattern);
    }
}

QRegExp QItemModelBarDataProxy::rotationRolePattern() const
{
    return dptrc()->m_rotationRolePattern;
}

/*!
 * \property QItemModelBarDataProxy::rowRoleReplace
 *
 * \brief The replace content to be used in conjunction with rowRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa rowRole, rowRolePattern
 */
void QItemModelBarDataProxy::setRowRoleReplace(const QString &replace)
{
    if (dptr()->m_rowRoleReplace != replace) {
        dptr()->m_rowRoleReplace = replace;
        emit rowRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::rowRoleReplace() const
{
    return dptrc()->m_rowRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::columnRoleReplace
 *
 * \brief The replace content to be used in conjunction with columnRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa columnRole, columnRolePattern
 */
void QItemModelBarDataProxy::setColumnRoleReplace(const QString &replace)
{
    if (dptr()->m_columnRoleReplace != replace) {
        dptr()->m_columnRoleReplace = replace;
        emit columnRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::columnRoleReplace() const
{
    return dptrc()->m_columnRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::valueRoleReplace
 *
 * \brief The replace content to be used in conjunction with valueRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa valueRole, valueRolePattern
 */
void QItemModelBarDataProxy::setValueRoleReplace(const QString &replace)
{
    if (dptr()->m_valueRoleReplace != replace) {
        dptr()->m_valueRoleReplace = replace;
        emit valueRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::valueRoleReplace() const
{
    return dptrc()->m_valueRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::rotationRoleReplace
 *
 * \brief The replace content to be used in conjunction with
 * rotationRolePattern.
 *
 * Defaults to empty string. For more information on how the search and replace using regular
 * expressions works, see QString::replace(const QRegExp &rx, const QString &after)
 * function documentation.
 *
 * \sa rotationRole, rotationRolePattern
 */
void QItemModelBarDataProxy::setRotationRoleReplace(const QString &replace)
{
    if (dptr()->m_rotationRoleReplace != replace) {
        dptr()->m_rotationRoleReplace = replace;
        emit rotationRoleReplaceChanged(replace);
    }
}

QString QItemModelBarDataProxy::rotationRoleReplace() const
{
    return dptrc()->m_rotationRoleReplace;
}

/*!
 * \property QItemModelBarDataProxy::multiMatchBehavior
 *
 * \brief How multiple matches for each row/column combination are handled.
 *
 * Defaults to QItemModelBarDataProxy::MMBLast. The chosen behavior affects both bar value
 * and rotation.
 *
 * For example, you might have an item model with timestamped data taken at irregular intervals
 * and you want to visualize total value of data items on each day with a bar graph.
 * This can be done by specifying row and column categories so that each bar represents a day,
 * and setting multiMatchBehavior to QItemModelBarDataProxy::MMBCumulative.
 */
void QItemModelBarDataProxy::setMultiMatchBehavior(QItemModelBarDataProxy::MultiMatchBehavior behavior)
{
    if (dptr()->m_multiMatchBehavior != behavior) {
        dptr()->m_multiMatchBehavior = behavior;
        emit multiMatchBehaviorChanged(behavior);
    }
}

QItemModelBarDataProxy::MultiMatchBehavior QItemModelBarDataProxy::multiMatchBehavior() const
{
    return dptrc()->m_multiMatchBehavior;
}

/*!
 * \internal
 */
QItemModelBarDataProxyPrivate *QItemModelBarDataProxy::dptr()
{
    return static_cast<QItemModelBarDataProxyPrivate *>(d_ptr.data());
}

/*!
 * \internal
 */
const QItemModelBarDataProxyPrivate *QItemModelBarDataProxy::dptrc() const
{
    return static_cast<const QItemModelBarDataProxyPrivate *>(d_ptr.data());
}

// QItemModelBarDataProxyPrivate

QItemModelBarDataProxyPrivate::QItemModelBarDataProxyPrivate(QItemModelBarDataProxy *q)
    : QBarDataProxyPrivate(q),
      m_itemModelHandler(new BarItemModelHandler(q)),
      m_useModelCategories(false),
      m_autoRowCategories(true),
      m_autoColumnCategories(true),
      m_multiMatchBehavior(QItemModelBarDataProxy::MMBLast)
{
}

QItemModelBarDataProxyPrivate::~QItemModelBarDataProxyPrivate()
{
    delete m_itemModelHandler;
}

QItemModelBarDataProxy *QItemModelBarDataProxyPrivate::qptr()
{
    return static_cast<QItemModelBarDataProxy *>(q_ptr);
}

void QItemModelBarDataProxyPrivate::connectItemModelHandler()
{
    QObject::connect(m_itemModelHandler, &BarItemModelHandler::itemModelChanged,
                     qptr(), &QItemModelBarDataProxy::itemModelChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rowRoleChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::columnRoleChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::valueRoleChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rotationRoleChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rowCategoriesChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::columnCategoriesChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::useModelCategoriesChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::autoRowCategoriesChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::autoColumnCategoriesChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rowRolePatternChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::columnRolePatternChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::valueRolePatternChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rotationRolePatternChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rowRoleReplaceChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::columnRoleReplaceChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::valueRoleReplaceChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::rotationRoleReplaceChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
    QObject::connect(qptr(), &QItemModelBarDataProxy::multiMatchBehaviorChanged,
                     m_itemModelHandler, &AbstractItemModelHandler::handleMappingChanged);
}

QT_END_NAMESPACE_DATAVISUALIZATION
