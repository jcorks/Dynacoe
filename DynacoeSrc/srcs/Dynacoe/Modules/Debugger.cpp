/*

Copyright (c) 2018, Johnathan Corkery. (jcorkery@umich.edu)
All rights reserved.

This file is part of the Dynacoe project (https://github.com/jcorks/Dynacoe)
Dynacoe was released under the MIT License, as detailed below.



Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished 
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.



*/

#include <Dynacoe/Modules/Debugger.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/BuiltIn/DataGrid.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Time.h>


#include <Dynacoe/BuiltIn/InputBox.h>
#include <sstream>
#include <queue>


using namespace Dynacoe;


#include <Dynacoe/Entity.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Modules/Input.h>

#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/Console.h>
#include <Dynacoe/Components/DataTable.h>

#include <cassert>


class DebugWindow : public Entity {
  public:
    DebugWindow();

    void Resize(int w, int h){}
    void Move(int x, int y){}

    int Width() {return w;}
    int Height(){return h;}

    void Follow(Entity::ID);

    void OnStep();

  private:
    Shape2D bg;

    Shape2D exit;
    GUI exitButton;

    Shape2D pause;
    GUI pauseButton;


    Text2D title;
    GUI titleBarGrab;


    Shape2D linkBar;
    Text2D link;

    GUI linkBackButton;
    Shape2D linkBack;
    Text2D linkBackText;


    Shape2D statusBar;
    Text2D  status;


    Shape2D attached;
    GUI attachedButton;
    Text2D attachedText;

    Shape2D variables;
    GUI variablesButton;
    Text2D variablesText;

    DataGrid * attachedTable;
    DataGrid * variablesTable;

    Entity * attachedTableGUIcontainer;
    Entity * variablesTableGUIcontainer;

    bool IsInWindowMode();

    int w;
    int h;

    Clock updateCycle;
    static DynacoeEvent(event_update_info);



    enum class PageMode {
        Attached,
        Variables
    };
    PageMode mode;
    void SetPageMode(PageMode);


    static DynacoeEvent(event_click_table_attached);

    static DynacoeEvent(event_click_back);
    static DynacoeEvent(event_hover_enter_back);
    static DynacoeEvent(event_hover_leave_back);

    static DynacoeEvent(event_hover_enter_attached);
    static DynacoeEvent(event_hover_leave_attached);
    static DynacoeEvent(event_click_attached);

    static DynacoeEvent(event_hover_enter_variables);
    static DynacoeEvent(event_hover_leave_variables);
    static DynacoeEvent(event_click_variables);

    static DynacoeEvent(event_hover_enter_exit);
    static DynacoeEvent(event_hover_leave_exit);
    static DynacoeEvent(event_click_exit);


    static DynacoeEvent(event_hover_enter_pause);
    static DynacoeEvent(event_hover_leave_pause);
    static DynacoeEvent(event_click_pause);


    static DynacoeEvent(event_drag_begin_window);
    bool isBeingDragged;

    void UpdateInfoAttached();
    void UpdateInfoVariables();


    void ForceUpdate();
    Entity::ID target;
};


using namespace std;

const int default_w_c = 400;
const int default_h_c = 300;



const int grab_bar_height_c        = 20;
const int exit_button_width_c      = 20;
const int pause_button_width_c     = 20;
const int window_button_width_c    = 20;
const int location_row_height_c    = 20;
const int back_button_width_c      = 20;
const int frame_width_c            = 5;
const int info_bar_height_c        = 15;
const int tab_bar_height_c         = 15;
const int entry_height_c           = 15;
const int update_cycle_ms_c        = 500;
const int scroll_button_width      = 5;

const int data_attached_ids_x       = 0;
const int data_attached_names_x     = 24;
const int data_attached_run_times_x = 100;
const int data_attached_draw_times_x= 120;




const Vector data_text_offset       (12, 4);



const Color bg_color                 ("#1A071096");
const Color exit_color               (210, 80, 80, 245);
const Color pause_color              (160, 160, 160, 255);
const Color window_color             (30, 30, 30, 255);
const Color location_color           (30, 30, 110, 178);
const Color back_color               (40, 40, 210, 178);
const Color scroll_color             (255, 255, 255, 40);
const Color info_color               (0, 0, 0, 255);
const Color tab_color                (30, 10, 70, 255);
const Color tab_inactive_color       (15, 5, 35, 255);
const Color inactive_color           (0, 0, 0, 128);
const Color ent_highlight_color      (255, 255, 0, 128);

const Color info_text_color          (120, 255, 120, 255);
const Color location_text_color      (80, 255, 200, 255);
const Color tab_text_color           (205, 205, 205, 255);

const Color data_title               (120, 120, 120, 255);
const Color data_text_world          (180, 225, 225, 255);
const Color data_text_entity         (180, 225, 100, 255);
const Color data_text_component      (225, 180, 180, 255);
const Color data_text_component_desc (150, 100, 100, 255);
const Color data_text_variable       (0, 255, 128, 255);
const Color data_text_variable_builtin(100, 255, 228, 255);


const Color data_row_even            (10, 10, 20, 255);
const Color data_row_odd             (15, 15, 25, 255);


const Color data_time_null           (64, 64, 64, 255);
const Color data_time_fair           (64, 255, 64, 255);
const Color data_time_poor           (255, 255, 64, 255);
const Color data_time_really_bad     (255, 0, 0, 255);
const Color data_time_frozen         (180, 220, 255, 255);

const Color highlight_add_color      (30, 30, 30, 0);

const char * attached_string_c      = "Children";
const char * variable_string_c      = "State";

const Vector text_offset_c = {5, 5};


DebugWindow::DebugWindow() {
    isBeingDragged = false;
    Vector base;

    // background
    base = {0, 0};
    //base = CreateChild<Entity>();
    bg.FormRectangle(default_w_c, default_h_c);
    bg.color = bg_color;
    AddComponent(&bg);

    // titlebar
    base = {0, 0};
    AddComponent(&title);
    title.node.Position() = base + text_offset_c;
    titleBarGrab.DefineRegion(default_w_c,
        grab_bar_height_c +
        location_row_height_c +
        info_bar_height_c
    );
    titleBarGrab.node.Position() = base;
    titleBarGrab.InstallHandler("on-click", event_drag_begin_window);
    AddComponent(&titleBarGrab);


    // exit
    exit.node.Position() = {default_w_c - exit_button_width_c, 0};
    exit.color = exit_color;
    exit.FormRectangle(exit_button_width_c, grab_bar_height_c);
    AddComponent(&exit);

    exitButton.node.Position() = exit.node.GetPosition();
    exitButton.DefineRegion(exit_button_width_c, grab_bar_height_c);
    exitButton.InstallHandler("on-click", event_click_exit);
    exitButton.InstallHandler("on-enter", event_hover_enter_exit);
    exitButton.InstallHandler("on-leave", event_hover_leave_exit);
    AddComponent(&exitButton);


    // pause
    pause.node.Position() = {default_w_c -2* exit_button_width_c, 0};
    pause.color = back_color;
    pause.FormRectangle(exit_button_width_c, grab_bar_height_c);
    AddComponent(&pause);

    pauseButton.node.Position() = pause.node.GetPosition();
    pauseButton.DefineRegion(exit_button_width_c, grab_bar_height_c);
    pauseButton.InstallHandler("on-click", event_click_pause);
    pauseButton.InstallHandler("on-enter", event_hover_enter_pause);
    pauseButton.InstallHandler("on-leave", event_hover_leave_pause);
    AddComponent(&pauseButton);


    // location / link
    base = {0, (float)grab_bar_height_c};
    AddComponent(&linkBar);
    linkBar.node.Position() = Vector(back_button_width_c, 0) + base;
    linkBar.FormRectangle(default_w_c - back_button_width_c, location_row_height_c);
    linkBar.color = location_color;
    AddComponent(&link);
    link.node.Position() = linkBar.node.GetPosition() + text_offset_c;

    linkBackButton.node.Position() = base;
    linkBackButton.DefineRegion(back_button_width_c, location_row_height_c);
    linkBackButton.InstallHandler("on-click", event_click_back);
    linkBackButton.InstallHandler("on-enter", event_hover_enter_back);
    linkBackButton.InstallHandler("on-leave", event_hover_leave_back);
    AddComponent(&linkBackButton);

    linkBack.node.Position() = base;
    linkBack.color = back_color;
    linkBack.FormRectangle(back_button_width_c, grab_bar_height_c);
    AddComponent(&linkBack);

    linkBackText.node.Position() = base + Vector(6, 6);
    linkBackText.text = "<";
    AddComponent(&linkBackText);


    // status bar
    base.y += grab_bar_height_c;
    statusBar.node.Position() = base;
    statusBar.FormRectangle(default_w_c, info_bar_height_c);
    statusBar.color = "#00000056";
    AddComponent(&statusBar);

    status.node.Position() = base;
    status.SetTextColor(info_text_color);
    status.SetFontSize(9);
    AddComponent(&status);




    // attached button
    base.y += info_bar_height_c;
    attached.node.Position() = base;
    attached.FormRectangle(default_w_c/2, tab_bar_height_c);
    attached.color = tab_color;
    AddComponent(&attached);

    attachedText.node.Position() = base;
    attachedText.text = attached_string_c;
    attachedText.SetTextColor(tab_text_color);
    AddComponent(&attachedText);

    attachedButton.node.Position() = base;
    attachedButton.DefineRegion(default_w_c/2, tab_bar_height_c);
    attachedButton.InstallHandler("on-click", event_click_attached);
    attachedButton.InstallHandler("on-enter", event_hover_enter_attached);
    attachedButton.InstallHandler("on-leave", event_hover_leave_attached);
    AddComponent(&attachedButton);

    variables.node.Position() = base + Vector(default_w_c/2, 0);
    variables.FormRectangle(default_w_c/2, tab_bar_height_c);
    variables.color = tab_color;
    AddComponent(&variables);

    variablesText.node.Position() = base + Vector(default_w_c/2, 0);
    variablesText.text = variable_string_c;
    variablesText.SetTextColor(tab_text_color);
    AddComponent(&variablesText);

    variablesButton.node.Position() = base + Vector(default_w_c/2, 0);
    variablesButton.DefineRegion(default_w_c, tab_bar_height_c);
    variablesButton.InstallHandler("on-click", event_click_variables);
    variablesButton.InstallHandler("on-enter", event_hover_enter_variables);
    variablesButton.InstallHandler("on-leave", event_hover_leave_variables);
    AddComponent(&variablesButton);


    node.Position() = {100, 100};

    base.y += tab_bar_height_c;
    attachedTable = CreateChild<DataGrid>();
    attachedTable->clickCallback = event_click_table_attached;
    attachedTable->node.Position() = base;
    attachedTable->titleColor = {.6f, .6f, .6f, 1.f};
    attachedTable->AddColumn("ID",          130);
    attachedTable->AddColumn("Name",        190);
    attachedTable->AddColumn("Step% Draw%", 80);
    attachedTable->draw = false;
    attachedTable->step = false;

    attachedTable->SetRowsVisible(17);

    attachedTableGUIcontainer = attachedTable->CreateChild<Entity>();
    for(uint32_t i = 0; i < attachedTable->GetRowsVisible(); ++i) {
        Entity * child = attachedTableGUIcontainer->CreateChild<Entity>();
        child->node.Position().y = (i+1)*attachedTable->RowHeight(); // +1 to skip titles

        DataTable * data = child->BindComponent<DataTable>();
        data->Write("index", i);

        child->SetName("RowObject");
    }
    SetName("DebugWindow");

    variablesTable = CreateChild<DataGrid>();
    variablesTable->node.Position() = base;
    variablesTable->titleColor = {.6f, .6f, .6f, 1.f};
    variablesTable->AddColumn("Name",            120, {.7f, .9f, .7f, 1.f});
    variablesTable->AddColumn("Value/State",     280);
    variablesTable->draw = false;
    variablesTable->step = false;
    variablesTable->SetRowsVisible(17);


    AddComponent(&updateCycle);
    updateCycle.Set(update_cycle_ms_c);
    updateCycle.InstallHandler("clock-expire", event_update_info);

    SetPageMode(PageMode::Attached);
    updateCycle.EmitEvent("clock-expire");

}


DynacoeEvent(DebugWindow::event_click_table_attached) {
    uint32_t row = Chain(args[0]).AsUInt32();

    // index refers to the viewing index, so we need the read id from it
    // we look into the displayed grid
    Entity * parent = self.Identify();
    while(parent && !dynamic_cast<DebugWindow*>(parent)) {
        parent = parent->HasParent() ? &parent->GetParent() : nullptr;
    }
    assert(parent);
    DebugWindow * grid = (DebugWindow*)parent;

    auto children = grid->target.Identify()->GetChildren();

    if (row >= children.size()) return true;
    grid->Follow(children[row]);
    return true;
}

DynacoeEvent(DebugWindow::event_click_back) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (!src->target.Valid()) return true;
    if (!src->target.Identify()->HasParent()) return true;

    src->Follow(src->target.Identify()->GetParent().GetID());
    return true;
}


DynacoeEvent(DebugWindow::event_hover_enter_back) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->linkBack.color = back_color + highlight_add_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_leave_back) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->linkBack.color = back_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_enter_attached) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (src->mode == PageMode::Attached)
        src->attached.color = tab_color + highlight_add_color;
    else
        src->attached.color = tab_inactive_color + highlight_add_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_leave_attached) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (src->mode == PageMode::Attached)
        src->attached.color = tab_color;
    else
        src->attached.color = tab_inactive_color;
    return true;
}

DynacoeEvent(DebugWindow::event_click_attached) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->SetPageMode(PageMode::Attached);
    return true;
}



DynacoeEvent(DebugWindow::event_hover_enter_variables) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (src->mode == PageMode::Variables)
        src->variables.color = tab_color + highlight_add_color;
    else
        src->variables.color = tab_inactive_color + highlight_add_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_leave_variables) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (src->mode == PageMode::Variables)
        src->variables.color = tab_color;
    else
        src->variables.color = tab_inactive_color;
    return true;
}

DynacoeEvent(DebugWindow::event_click_variables) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->SetPageMode(PageMode::Variables);
    return true;
}




DynacoeEvent(DebugWindow::event_hover_enter_exit) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->exit.color = exit_color + highlight_add_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_leave_exit) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->exit.color = exit_color;
    return true;
}

DynacoeEvent(DebugWindow::event_click_exit) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->Remove();
    return true;
}



DynacoeEvent(DebugWindow::event_hover_enter_pause) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->pause.color = back_color + highlight_add_color;
    return true;
}

DynacoeEvent(DebugWindow::event_hover_leave_pause) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->pause.color = back_color;
    return true;
}

DynacoeEvent(DebugWindow::event_click_pause) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    if (!src->target.Valid()) return true;
    src->target.Identify()->step = !src->target.Identify()->step;
    return true;
}


DynacoeEvent(DebugWindow::event_drag_begin_window) {
    DebugWindow * src = self.IdentifyAs<DebugWindow>();
    src->isBeingDragged = true;
    return true;
}



void DebugWindow::Follow(Entity::ID id) {
    target = id;
    ForceUpdate();
}

void DebugWindow::ForceUpdate() {
    updateCycle.EmitEvent("clock-expire");
}


void DebugWindow::SetPageMode(PageMode m) {
    mode = m;
    variablesTable->draw = false;
    variablesTable->step = false;
    attachedTable->draw = false;
    attachedTable->step = false;

    variables.color = tab_inactive_color;
    attached.color = tab_inactive_color;

    switch(m) {
      case PageMode::Attached:
        attachedTable->draw = true;
        attachedTable->step = true;
        attached.color = tab_color;
        break;

      case PageMode::Variables:
        variablesTable->draw = true;
        variablesTable->step = true;
        variables.color = tab_color;
        break;
    }


}

void DebugWindow::UpdateInfoAttached() {
    // gather all things attached to current
    attachedTable->Clear();

    if (!target.Valid()) {
        target = Engine::Root();
    }

    if (!target.Valid()) {
        title.text = "Nothing";
        link.text = "???";
        status.text = "The engine has no objects attached.";
        return;
    }

    // title
    title.text = Chain() << target.Identify()->GetName() << " - ID: " << target.String();


    // form path in tree
    link.text = "";
    Entity::ID temp = target;
    while(temp.Valid()) {
        link.text = temp.Identify()->GetName() + "/" + link.text;
        if (temp.Identify()->HasParent()) {
            temp = temp.Identify()->GetParent().GetID();
        } else {
            temp = Entity::ID();
        }
    }


    // populate entities first
    auto ents = target.Identify()->GetChildren();

    status.text = (Chain() << ents.size() << " Child Entities");


    for(uint32_t i = 0; i < ents.size(); ++i) {
        if (i >= attachedTable->GetRowCount()) {
            attachedTable->AddRow();
        }
        attachedTable->Get(0, i) = (Chain() << ents[i].String());
        attachedTable->Get(1, i) = ents[i].Identify()->GetName();
    }




}

void DebugWindow::UpdateInfoVariables() {
    // gather all things attached to current
    variablesTable->Clear();

    if (!target.Valid()) return;
    Entity * e = target.Identify();
    auto vars = e->GetWatchedVars();

    uint32_t lineIndex = 0;
    for(size_t i = 0; i < vars.size(); ++i) {
        if (lineIndex <= variablesTable->GetRowCount())
            variablesTable->AddRow();

        variablesTable->Get(0, lineIndex) = vars[i].GetName();
        variablesTable->Get(1, lineIndex) = vars[i].ToString();

        lineIndex++;
    }

    auto components = e->GetComponents();
    if (components.size() && vars.size()) {
        variablesTable->AddRow();
        variablesTable->Get(0, lineIndex) = "________________________";
        lineIndex++;
    }
    for(size_t i = 0; i < components.size(); ++i) {
        while (lineIndex >= variablesTable->GetRowCount())
            variablesTable->AddRow();

        variablesTable->Get(0, lineIndex) = components[i]->GetTag();


        // add new rows for multiline info
        std::string data = components[i]->GetInfo();
        std::string * line = &variablesTable->Get(1, lineIndex);
        *line = "";
        for(size_t n = 0; n < data.length(); ++n) {
            if (data[n] == '\n') {
                lineIndex++;
                if (lineIndex <= variablesTable->GetRowCount())
                    variablesTable->AddRow();
                line = &variablesTable->Get(1, lineIndex);
                *line = "";
            } else {
                *line += data[n];
            }
        }


        lineIndex+=2;
    }
}


DynacoeEvent(DebugWindow::event_update_info) {
    DebugWindow * w = self.IdentifyAs<DebugWindow>();
    if (w->mode == PageMode::Attached)
        w->UpdateInfoAttached();
    else
        w->UpdateInfoVariables();
    w->updateCycle.Set(update_cycle_ms_c);
    return true;
}






void DebugWindow::OnStep() {
    if (isBeingDragged) {
        if (!Input::GetState(MouseButtons::Left)) {
            isBeingDragged = false;
            return;
        }
        node.Position() += {
            (float)Input::MouseXDelta(),
            (float)Input::MouseYDelta()
        };
    }
}


























const int slowestCount = 40;

static Clock updateTimer;
static std::vector<Entity::ID> slowestIDs;
static std::vector<double> slowestTimes;


#include <Dynacoe/Components/Text2D.h>
#include <Dynacoe/Components/Mutator.h>
#include <Dynacoe/Components/StateControl.h>

using namespace Dynacoe;


static float lastDrawTime;
static float lastRunTime;
static float lastDebugTime;
static float lastSysTime;
static float timeSinceEscHeld;

static std::string lowerString(const std::string & str) {
    static std::string out;
    out = str;
    for(uint32_t i = 0; i < str.size(); ++i) {
        out[i] = tolower(str[i]);
    }
    return out;
}


static std::vector<Entity::ID> GuessByName(const std::string & name, int max) {
    std::vector<Entity::ID> results;


    std::string nonCase = lowerString(name);

    // for each world, gather all ents and check name.

    std::vector<Entity::ID> all = Entity::GetAll();
    Entity * curEnt;
    for(uint32_t i = 0; i < all.size() && (results.size() < max); ++i) {
        curEnt = all[i].Identify();

        if (!curEnt) continue;

        // if substring matches non-case sensitively, it's a match
        if (strstr(lowerString(curEnt->GetName()).c_str(), nonCase.c_str())) {
            results.push_back(curEnt->GetID());
        }

    }
    return results;

}





class ProcessTimeFeedback : public Dynacoe::Entity {
  public:
    Text2D timeString;
    Text2D titleString;

    Shape2D drawRect;
    Shape2D runRect;
    Shape2D debugRect;
    Shape2D sysRect;

    Shape2D barBG;
    Shape2D barDraw;
    Shape2D barRun;
    Shape2D barSys;
    Shape2D barDebug;

    Node * barRunTransform;
    Node * barSysTransform;
    Node * barDebugTransform;

    Color drawColor;
    Color runColor;
    Color sysColor;
    Color bgColor;
    Color debugColor;

    Vector legendSquareOrigin;
    Vector graphOrigin;
    float graphWidth;
    Vector textOrigin;
    ProcessTimeFeedback() {
        legendSquareOrigin = {4, 32};
        graphOrigin = Vector(0, 12);
        graphWidth = 120;
        textOrigin = Vector(10, 32);


        drawColor  = Color   (255,     128,    64,   255);
        runColor   = Color   (64,      255,   128,   255);


        sysColor   = Color   (64,      128,   255,   255);
        debugColor = Color   (255,     255,   128,   255);
        bgColor    = Color   (32,      32,     32,   255);




        float pool = 1000.0 / Engine::GetMaxFPS(); // in ms


        Entity * board = CreateChild<Entity>();

        //Graphics::DrawString(timing.c_str() , timingOrigin, Color(255, 255, 255, 255));
        timeString.node.Position() = textOrigin;
        titleString.node.Position() = {0, 0};
        timeString.SetTextColor("white");
        board->AddComponent(&timeString);
        board->AddComponent(&titleString);

        // legend
        drawRect.node.Position() = legendSquareOrigin;
        drawRect.FormRectangle(2, 12);
        drawRect.color = drawColor;
        board->AddComponent(&drawRect);

        runRect.node.Position() = legendSquareOrigin + Vector{0, 12};
        runRect.FormRectangle(2, 12);
        runRect.color = runColor;
        board->AddComponent(&runRect);

        sysRect.node.Position() = legendSquareOrigin +Vector{0, 24};
        sysRect.FormRectangle(2, 12);
        sysRect.color = sysColor;
        board->AddComponent(&sysRect);

        debugRect.node.Position() = legendSquareOrigin +Vector{0, 36};
        debugRect.FormRectangle(2, 12);
        debugRect.color = debugColor;
        board->AddComponent(&debugRect);

        // graph
        float graphWidth = 100;
        // draw + run + sys + free = total time
        float drawWidth = (lastDrawTime /pool) * graphWidth;
        float runWidth =  (lastRunTime / pool) * graphWidth;
        float sysWidth =  (lastSysTime / pool) * graphWidth;
        float debugWidth =(lastDebugTime/pool) * graphWidth;



        float runXPos = drawWidth;
        float sysXPos = runXPos + runWidth;
        float debugXPos = sysXPos + sysWidth;




        barBG.FormRectangle(graphWidth + 2, 12);
        barBG.node.Position() = graphOrigin + Vector(-1, -1);
        barBG.color = bgColor;
        board->AddComponent(&barBG);

        barDraw.node.Position() = graphOrigin;
        barDraw.color = drawColor;
        board->AddComponent(&barDraw);


        barRunTransform = &barRun.node;
        barRun.color = runColor;
        board->AddComponent(&barRun);

        barSysTransform = &barSys.node;
        barSys.color = sysColor;
        board->AddComponent(&barSys);

        barDebugTransform = &barDebug.node;
        barDebug.color = debugColor;
        board->AddComponent(&barDebug);
    }

    void OnStep() {
        float pool = 1000.0 / Engine::GetMaxFPS(); // in ms
        titleString.text = Chain() << "Frame Usage (" << Engine::GetDiagnostics().currentFPS << " FPS)";




       // draw + run + sys + free = total time
       float drawWidth = (lastDrawTime /pool) * graphWidth;
       float runWidth =  (lastRunTime / pool) * graphWidth;
       float sysWidth =  (lastSysTime / pool) * graphWidth;
       float debugWidth =(lastDebugTime/pool) * graphWidth;

       int remaining = (int)(((pool - (lastDrawTime + lastRunTime + lastSysTime + lastDebugTime)) / pool) * 100);




       float runXPos = drawWidth;
       float sysXPos = runXPos + runWidth;
       float debugXPos = sysXPos + sysWidth;


       timeString.text = (Chain()
              << "Draw     : " << (int)(lastDrawTime * 100 / pool) << "%\n"
              << "Step     : " << (int)(lastRunTime * 100 / pool) << "%\n"
              << "Engine   : " << (int)(lastSysTime * 100 / pool)  << "%\n"
              << "System   : " << (int)(lastDebugTime * 100 / pool)  << "%\n"
              << "Remaining: " << (remaining < 0 ? Chain() << "OVERLOAD": Chain() << remaining) << "%");


       // todo: dont allow overflow drawing
       barDraw.FormRectangle(drawWidth  , 10);

       barRun.FormRectangle(runWidth  , 10);
       barRunTransform->Position() = graphOrigin + Vector(runXPos,   0);

       barSys.FormRectangle(sysWidth  , 10);
       barSysTransform->Position() = graphOrigin + Vector(sysXPos,   0);

       barDebug.FormRectangle(debugWidth  , 10);
       barDebugTransform->Position() = graphOrigin + Vector(debugXPos,   0);


    }


};







class DebuggerBase : public Entity {
  public:


    Text2D versionLabel; Text2D version;
    Text2D mouseLabel;   Text2D mouseXY;
    /*
    DBGData version;
    DBGData mouseXY;*/
    Shape2D bg;

    Entity * overview;
    Entity * entity;
    Entity * vars;

    const int width = 150;
    const float slide_duration_s = .2f;


    enum class Page {
        Overview,
        Entity,
        Vars
    };

    std::vector<Entity::ID> overviewIDs;
    std::vector<Entity::ID> searchIDs;

    static DynacoeEvent(click_row_time) {
        uint32_t row = (Chain() << args[0]).AsUInt32();

        DebuggerBase * base = dynamic_cast<DebuggerBase*>(&self.Identify()->GetParent().GetParent());
        DebugWindow * w = base->CreateChild<DebugWindow>();
        w->Follow(base->overviewIDs[row]);

        return true;
    }

    static DynacoeEvent(click_row_search) {
        uint32_t row = (Chain() << args[0]).AsUInt32();

        DebuggerBase * base = dynamic_cast<DebuggerBase*>(&self.Identify()->GetParent().GetParent());
        DebugWindow * w = base->CreateChild<DebugWindow>();
        w->Follow(base->searchIDs[row]);

        return true;
    }


    void SwitchPage(Page page) {
        overview->draw = false;
        overview->step = false;
        entity->draw = false;
        entity->step = false;
        vars->draw = false;
        vars->step = false;

        overviewText->SetTextColor("#A0A0A0");
        entityText->SetTextColor("#A0A0A0");
        varsText->SetTextColor("#A0A0A0");

        switch(page) {
          case Page::Overview:
            overview->draw = true;
            overview->step = true;
            overviewText->SetTextColor("#AAFFAA");
            break;
          case Page::Entity:
            entity->draw = true;
            entity->step = true;
            entityText->SetTextColor("#AAFFAA");
            break;
          case Page::Vars:
            vars->draw = true;
            vars->step = true;
            varsText->SetTextColor("#AAFFAA");
            break;

        }
    }

    StateControl state;
    Mutator mutate;

    DebuggerBase() {
        SetName("Debugger");
        lastDrawTime = 0.f;
        lastRunTime =  0.f;
        lastDebugTime =0.f;
        lastSysTime =  0.f;
        updateTimer.Set(1000);

        AddComponent(&state);
        AddComponent(&mutate);

        state.CreateState("slide-in",  {State_SlideIn_Step, nullptr, State_SlideIn_Init});
        state.CreateState("slide-out", {State_SlideOut_Step, nullptr, State_SlideOut_Init});


        CreateButtons();
        CreateOverviewPage();
        CreateEntityPage();
        CreateVarsPage();
        CreateBase();

        SwitchPage(Page::Overview);

        bg.FormRectangle(width, 300);
        bg.color = "#1A0710";
        bg.color.a = .6f;

        show = false;
        draw = show;
        step = show;

        node.Position().x = -width;
    }

    void CreateBase() {
        Entity * bgBase = CreateChild<Entity>();
        bgBase->AddComponent(&bg);
        //bgBase->SetPriorityFirst();
    }

    std::string lastText;
    void Toggle() {
        show = !show;
        if (show) {
            step = true;
            draw = true;
            state.Execute("slide-in");
        } else {
            state.Execute("slide-out");
        }
        UpdateCycle();
    }


    void OnStep() {
        mouseXY.text = (Chain() << Input::MouseX() << ", " << Input::MouseY());
        hover->node.Position().x = Mutator::StepTowards(hover->node.GetPosition().x, hoverX);
        if (updateTimer.IsExpired()) {
            UpdateCycle();
            updateTimer.Set(1500);
        }
    }



    GUI * overviewButton;
    GUI * entityButton;
    GUI * varsButton;

    Text2D * overviewText;
    Text2D * entityText;
    Text2D * varsText;
    static DynacoeEvent(ButtonClickEvent) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();



        if (component == base->overviewButton) {
            base->SwitchPage(Page::Overview);
        } else if (component == base->entityButton) {
            base->SwitchPage(Page::Entity);
        } else {
            base->SwitchPage(Page::Vars);
        }
        return true;

    }

    Shape2D * hover;
    int acc;
    float hoverX;
    static DynacoeEvent(ButtonEnterEvent) {
        GUI * gui = dynamic_cast<GUI*>(component);
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        base->acc++;
        base->hover->draw = (base->acc > 0);
        base->hoverX = gui->node.Position().x;
        return true;
    }

    static DynacoeEvent(ButtonLeaveEvent) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        base->hover->draw = false;
        base->acc--;
        base->hover->draw = (base->acc > 0);
        return true;
    }



    void CreateButtons() {
        Entity * buttons = CreateChild<Entity>();
        overviewText    = new Text2D("Perf", "#A0A0A0");
        entityText      = new Text2D("Entity", "#A0A0A0");
        varsText        = new Text2D("Vars", "#A0A0A0");
        Shape2D * h     = new Shape2D();

        h->FormRectangle(50, 1);
        h->color = "#AAFFAA";

        overviewText->node.Position() = {0, 0};
        entityText->node.Position()   = {49, 0};
        varsText->node.Position()     = {99, 0};

        buttons->AddComponent(overviewText);
        buttons->AddComponent(entityText);
        buttons->AddComponent(varsText);
        buttons->AddComponent(h);
        hover = h;
        hover->draw = false;
        hover->node.Position() = {0, 15};
        acc = 0;

        overviewButton = new GUI;
        entityButton = new GUI;
        varsButton = new GUI;

        overviewButton->DefineRegion(50, 20);
        entityButton->  DefineRegion(50, 20);
        varsButton->    DefineRegion(50, 20);

        overviewButton->node.Position() = {0, 0};
        entityButton->node.Position() = {50, 0};
        varsButton->node.Position() = {100, 0};

        overviewButton->InstallHandler("on-click", ButtonClickEvent);
        overviewButton->InstallHandler("on-enter", ButtonEnterEvent);
        overviewButton->InstallHandler("on-leave", ButtonLeaveEvent);

        entityButton->InstallHandler("on-click", ButtonClickEvent);
        entityButton->InstallHandler("on-enter", ButtonEnterEvent);
        entityButton->InstallHandler("on-leave", ButtonLeaveEvent);

        varsButton->InstallHandler("on-click", ButtonClickEvent);
        varsButton->InstallHandler("on-enter", ButtonEnterEvent);
        varsButton->InstallHandler("on-leave", ButtonLeaveEvent);


        AddComponent(overviewButton);
        AddComponent(entityButton);
        AddComponent(varsButton);
    }





    void CreateOverviewPage() {

        overview = CreateChild<Entity>();
        overview->node.Position() = Vector(0, 20);


        const float label_column_c = 0.f;
        const float data_column_c = 80;
        const float line_height_c = 10.f;

        ProcessTimeFeedback * p = overview->CreateChild<ProcessTimeFeedback>();
        p->node.Position() = {10, 0};




        table = overview->CreateChild<DataGrid>();
        table->node.Position() = {0, 100};
        for(int i = 0; i < slowestCount; ++i)
            table->AddRow();

        table->clickCallback = (click_row_time);

        table->AddColumn("Time", 36);
        table->AddColumn("Name", 114);
        table->SetRowsVisible(14);

    }


    InputBox * search;
    DataGrid * searchResults;
    void CreateEntityPage() {
        entity = CreateChild<Entity>();
        entity->node.Position() = {0, 20};
        search = entity->CreateChild<InputBox>();
        search->Resize(width, 12);


        searchResults = entity->CreateChild<DataGrid>();
        searchResults->clickCallback = click_row_search;
        searchResults->SetRowsVisible(18);
        searchResults->AddColumn("Name", width);
        for(int i = 0; i < 100; ++i) {
            searchResults->AddRow();
        }
        searchResults->node.Position() = {0, 40};
        search->SetText("Search...");
    }


    void CreateVarsPage() {
        vars = CreateChild<Entity>();
    }


    void UpdateCycle() {
        Entity * w = nullptr;



        lastDrawTime = Engine::GetDiagnostics().drawTimeMS;
        lastRunTime =  Engine::GetDiagnostics().stepTimeMS;
        lastSysTime =  Engine::GetDiagnostics().systemTimeMS;
        lastDebugTime =Engine::GetDiagnostics().engineRealTimeMS - (lastDrawTime + lastRunTime + lastSysTime);


        // record the top offenders of time cost

        if (overview->draw) {
            table->Clear();
            std::vector<Entity::ID> ids = Entity::GetAll();
            overviewIDs.clear();
            std::map<double, Entity::ID, std::greater<double>> times;

            slowestTimes.clear();
            slowestIDs.clear();
            double timeTotal;
            Entity * cur;
            for(int i = 0; i < ids.size(); ++i) {
                //assert(ids[i].Valid());
                cur = ids[i].Identify();
                if (!cur->IsStepping() && !cur->IsDrawing()) continue;
                timeTotal = cur->StepDuration() + cur->DrawDuration();
                if (timeTotal <= 0) continue;
                times[timeTotal] = ids[i];
            }
            auto it = times.begin();

            for(int i = 0; i < slowestCount, it != times.end(); ++it, i++) {
                table->AddRow();
                overviewIDs.push_back(it->second);
                slowestTimes.push_back(it->first);
                slowestIDs.  push_back(it->second);
                table->Get(0, i) = (Chain() << (int) ((it->first / (lastDrawTime+lastRunTime))*100) << "%");

                if (it->second.Identify()) {
                    table->Get(1, i) = it->second.Identify()->GetName();

                    table->GetTooltip(i) = Chain() <<
                        "Entity Info:\n"
                        "ID:       " << it->second.String() << "\n"
                        "Position: " << it->second.Identify()->node.GetPosition()
                    ;
                }

            }
        }

        // if on the entity tab, search for the string given
        if (search->GetText().size() && entity->draw) {
            searchResults->Clear();
            auto results = GuessByName(search->GetText(), 100);
            for(uint32_t i = 0; i < results.size(); ++i) {
                searchResults->AddRow();
                Entity * ent = results[i].Identify();
                if (!ent) continue;
                searchResults->Get(0, i) = ent->GetName();
                searchResults->GetTooltip(i) = Chain() <<
                    "Entity Info:\n"
                    "ID:       " << ent->GetID().String() << "\n"
                    "Position: " << ent->node.GetPosition()
                ;
            }
            searchIDs = results;
        }

    }




    static DynacoeEvent(State_SlideIn_Init) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        base->mutate.Clear(-base->width);
        base->mutate.NewMutation(base->slide_duration_s, 0, Mutator::Function::Logarithmic);
        base->mutate.Start();
        base->mutate.Bind(base->node.Position().x);
        return true;
    }

    static DynacoeEvent(State_SlideIn_Step) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        if (base->mutate.Expired()) {
            base->mutate.Unbind(base->node.Position().x);
            base->node.Position().x = 0;
            base->state.Halt();
        }
        return true;
    }



    static DynacoeEvent(State_SlideOut_Init) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        base->mutate.Clear(base->node.GetPosition().x);
        base->mutate.NewMutation(base->slide_duration_s, base->node.GetPosition().x-base->width, Mutator::Function::Quadratic);
        base->mutate.Start();
        base->mutate.Bind(base->node.Position().x);
        return false;
    }

    static DynacoeEvent(State_SlideOut_Step) {
        DebuggerBase * base = self.IdentifyAs<DebuggerBase>();
        if (base->mutate.Expired()) {
            base->mutate.Unbind(base->node.Position().x);
            base->draw = false;
            base->step = false;
            base->node.Position().x = -base->width;
            base->state.Halt();
        }
        return false;
    }


    DataGrid * table;
  private:
    bool show;

};





void Debugger::Init() {

}

void Debugger::InitAfter() {
    Entity::ID base = Entity::Create();
    debuggerBase = base.Identify()->CreateChild<DebuggerBase>()->GetID();
    Engine::AttachManager(base);
}

Backend * Debugger::GetBackend() {
    return nullptr;
}

void Debugger::RunBefore() {
    debuggerBase.Identify()->GetParent().node.Position() = Graphics::GetCamera2D().node.GetPosition();
    if (Input::IsPressed(Keyboard::Key_F3)) {
        debuggerBase.IdentifyAs<DebuggerBase>()->Toggle();
    }
}


void Debugger::RunAfter(){

}

void Debugger::DrawBefore(){}
void Debugger::DrawAfter() {
}
