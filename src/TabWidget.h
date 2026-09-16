#pragma once
#include <QTabWidget>
#include <QTabBar>
#include "EditorBase.h"

// Custom tab widget that manages EditorBase instances
// Shows modified indicator (•) in tab title
class TabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit TabWidget(QWidget* parent = nullptr);

    // Add an editor as a new tab; returns the new tab index
    int addEditor(EditorBase* editor);

    // Returns the EditorBase for the given tab index (nullptr if not found)
    EditorBase* editorAt(int index) const;

    // Returns the currently active EditorBase (nullptr if none)
    EditorBase* currentEditor() const;

    // Update the tab label to reflect the editor's display name + modified state
    void updateTabLabel(EditorBase* editor);

    // Remove the tab for this editor
    void removeEditor(EditorBase* editor);

    // Pinned editors are restored by MainWindow on the next application launch.
    bool isEditorPinned(EditorBase* editor) const;
    void setEditorPinned(EditorBase* editor, bool pinned);
    QList<EditorBase*> pinnedEditors() const;

signals:
    void editorChanged(EditorBase* editor);  // emitted when active tab changes
    void editorCloseRequested(EditorBase* editor);
    void editorPinRequested(EditorBase* editor, bool pinned);
    void pinnedEditorsChanged();

private slots:
    void onTabCloseRequested(int index);
    void onCurrentChanged(int index);
    void onTabContextMenuRequested(const QPoint& pos);
    void onTabMoved(int from, int to);

private:
    // Maps editor pointer -> tab index (maintained manually)
    QList<EditorBase*> m_editors;  // parallel to tab indices
    QList<EditorBase*> m_pinnedEditors;
};
