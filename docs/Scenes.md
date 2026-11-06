# Scenes

A **Scene** is a named group of widgets with its own root Lasagna, belonging to one window.
Use scenes to swap screens at runtime and to build HUDs such as menu bars and status bars.

The hierarchy is `Window / Scene / Container / Widgets`.

## Creating a scene

`SceneLoader` creates and registers scenes; **you own what it hands back**.

```cpp
#include "SceneLoader.hpp"

std::unique_ptr<RetroFuturaGUI::Scene> scene
{
    RetroFuturaGUI::SceneLoader::CreateScene("MainScene", window)  // nullptr if the name is taken
};

if(!scene)
    return;

//A scene starts empty - give it a root Lasagna before attaching anything
RetroFuturaGUI::AxisDefinition axis
{
    ._RowDefinition    = { 0.2f, 0.8f },
    ._ColumnDefinition = { 1.0f },
    ._LayerDefinition  = { 1.0f }
};

scene->SetLasagnaAxis(axis, window->GetProjection());
```

The root can also be an Environment, a free 3D room instead of cells, a ScrollView, so the whole
scene scrolls, or a WindowTab, so other windows dock into the scene as tabs. Any root container is
built through the same template, with whatever its constructor takes after the window:

```cpp
RetroFuturaGUI::Environment* room { scene->SetRootContainer<RetroFuturaGUI::Environment>(window->GetProjection()) };
RetroFuturaGUI::WindowTab* tabs { scene->SetRootContainer<RetroFuturaGUI::WindowTab>(window->GetProjection()) };
tabs->AddWindow(otherWindow);   //its native window hides, and this window draws it in the selected tab
```

Keep the `unique_ptr` alive for as long as you look the scene up by name - the loader only
borrows a pointer to it.

## Filling it

Widgets are parented to the scene's root Lasagna and attached into its cells. You own the
widgets; the Lasagna only arranges them.

```cpp
RetroFuturaGUI::Lasagna* root { scene->GetRootContainer<RetroFuturaGUI::Lasagna>() };   //nullptr if the root is another kind

auto button = std::make_unique<RetroFuturaGUI::Button>("Ok", window->GetProjection(),
                  root, RetroFuturaGUI::WidgetTypeID::Lasagna, window->GetGlfwWindow());

root->AddWidget(1, 0, 0, button.get(), RetroFuturaGUI::SizingMode::FIXED);
```

## Drawing it

```cpp
window->AddScene(scene.get());
```

**Order matters.** Scenes draw in the order they are added, and docked scenes reserve their
space in that same order, so add your menu bar before the content it sits above.

## Docked or overlay

An **overlay** scene is the default: it covers the whole client area and reserves nothing.
Use it for floating panels and modal dialogs.

A **docked** scene claims a strip along one edge. Everything laid out after it is fitted
around that strip, so a menu bar can neither hide nor swallow clicks meant for the content
behind it.

```cpp
menuBarScene->SetReservedEdge(RetroFuturaGUI::DockEdge::Top, 40.0f);
menuBarScene->ClearReservedEdge();   //back to an overlay
```

Note that an overlay scene does **not** block clicks reaching the scene beneath it - widgets
hit-test themselves, so there is no topmost-wins arbitration. For a modal, disable the lower
scene's widgets, passing `emitSignal = false` so the sweep does not fire every connected slot.

## Active vs enabled

Two different axes, deliberately:

| | controls | a "no" means |
|---|---|---|
| `Scene::SetActive` | whether the scene draws | it is not drawn and not interactive |
| `IWidget::SetEnabled` | whether a widget reacts, and its `ColorState` | it still draws, greyed out |

So an *active* scene full of *disabled* widgets is exactly the dimmed-behind-a-modal case.
An inactive scene also reserves no space, and is laid out again when reactivated.

Several scenes are active at once - there is no single "current" scene.

A third switch only matters for a window docked into a WindowTab: a scene marked
`ShowInTab(false)` hides, and gives back its strip, while its window sits in a tab. Meant for
the window's own bar, since the tab's field shows the window's title and close button instead.

```cpp
menuBarScene->ShowInTab(false);
```

## Closing a scene

Closing is deferred, which makes it safe to call from a button handler.

```cpp
RetroFuturaGUI::SceneLoader::CloseScene("MenuBarScene");   //marks it; nothing is freed yet
```

Two things must be in place for this to work:

**1. Drain once per frame**, after the windows have drawn:

```cpp
while(!WindowShouldClose())
{
    window->Draw();
    RetroFuturaGUI::SceneLoader::DrainPending();
}
```

`DrainPending` removes each marked scene from its window's draw order - which relayouts, so a
docked scene's strip is handed back - and drops its registry entry.

**2. Register a release hook** so closing also frees the scene, rather than only detaching it:

```cpp
RetroFuturaGUI::SceneLoader::SetReleaseHook(
    RetroFuturaGUI::SceneLoader::GetSceneID("MenuBarScene"),
    [this]() { _myScene.reset(); });   //whatever owns the unique_ptr drops it
```

Without a hook a closed scene stops drawing but stays in memory.

If you destroy a scene's owner directly instead - at shutdown, say - call
`SceneLoader::UnregisterScene(id)` first, or the registry is left holding a freed pointer.

## Ownership at a glance

| thing | owned by |
|---|---|
| `Scene` | whoever called `CreateScene` |
| the widgets in it | whoever created them |
| the root container (Lasagna, Environment, ScrollView or WindowTab) | the `Scene` |
| the windows docked into a WindowTab | whoever created them - the WindowTab only borrows them |
| the registry entry | `SceneLoader` (borrows the scene) |
| the draw order | `Window` (borrows the scene) |

A scene belongs to one window and cannot be shared between two - see the README's Scene
section for why, and for what a shared cross-window scene would require.
