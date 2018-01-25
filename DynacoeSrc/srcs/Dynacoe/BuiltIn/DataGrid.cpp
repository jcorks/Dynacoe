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

#include <Dynacoe/BuiltIn/DataGrid.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Components/Mutator.h>
using namespace Dynacoe;

DynacoeEvent(DataGrid::master_click) {
    DataGrid * grid = self.IdentifyAs<DataGrid>();
    if (grid->clickCallback) {
        uint32_t i = grid->rowmap[(GUI*)component];
        i--;

        i += grid->viewIndex;
        std::vector<std::string> argsH = {(Chain() << i)};
        grid->clickCallback(component, self, source, argsH);
    }
    return true;
}


DynacoeEvent(DataGrid::hover_enter) {
    DataGrid * grid = self.IdentifyAs<DataGrid>();

    // dont make it seem clickable if it isnt
    if (!grid->clickCallback) return true;
    uint32_t i = grid->rowmap[(GUI*)component];
    i--;
    // dont highlight if no actual data
    if (i + grid->viewIndex >= grid->rowCount) {
        return true;
    }

    grid->visibleRows[i+1]->color += Color{.2f, .2f, .2f, 0.f};
    return true;
}

DynacoeEvent(DataGrid::hover_leave) {
    DataGrid * grid = self.IdentifyAs<DataGrid>();
    uint32_t i = grid->rowmap[(GUI*)component];
    i--;

    // dont make it seem clickable if it isnt
    grid->visibleRows[i+1]->color = ((i+1)%2 == 0 ? grid->backgroundEvenColor : grid->backgroundOddColor);
    return true;
}


Dynacoe::DataGrid::DataGrid() {
    isDragging = false;
    backgroundEvenColor = "#101010";
    backgroundOddColor  = "#202020";
    titleColor          = "#FFCC80";
    clickCallback = nullptr;

    /*
    Shape2D * titleBG = new Shape2D();
    titleBG->FormRectangle(0, 0);
    titleBG->color = backgroundOddColor;
    visibleRows.push_back(titleBG);
    AddComponent(titleBG);
    */

    rowCount = 0;
    spanWidth = 0;
    viewIndex = 0;
    SetRowsVisible(10);
    AddComponent(&wheelZone);
    AddComponent(&scroll, UpdateClass::After);
    AddComponent(&scrollGrab);

    scrollGrab.InstallHandler("on-click", begin_scroll_drag);

    scrollAlpha = 0;
    scroll.color = {.5f, .25f, 1.f, 1.f};
    SetName("DataGrid");
}

void Dynacoe::DataGrid::AddColumn(const std::string & title, int width, const Color & color) {
    Column c;
    c.title = title; //new Text2D(title, real_titleColor);
    c.color = color;
    c.width = width;
    if (columns.size())
        c.x = columns[columns.size()-1].x + columns[columns.size()-1].width;
    else
        c.x = 0;

    //AddComponent(c.title);
    Text2D * next;
    spanWidth += c.width;
    for(uint32_t i = 0; i < rowCount; ++i) {
        c.rows.push_back("");
    }
    columns.push_back(c);
    for(uint32_t i = 0; i < visibleRows.size(); ++i) {
        visibleRows[i]->FormRectangle(spanWidth, lineHeight);
        visibleGUI[i]->DefineRegion(spanWidth, lineHeight);

        next = new Text2D;
        next->node.local.position.y = (i)*(lineHeight);
        next->node.local.position.x = c.x;
        next->SetTextColor(color);
        AddComponent(next);
        visibleText[i].push_back(next);
    }

    //RefreshView();
}

void Dynacoe::DataGrid::SetColumnWidth(uint32_t in, uint32_t w) {
    spanWidth = 0;
    for(uint32_t i = 0; i < columns.size(); ++i) {
        if (i == in) {
            columns[i].width = w;
        }
        columns[i].x = spanWidth;
        spanWidth += columns[i].width;
    }
    for(uint32_t i = 0; i < visibleRows.size(); ++i) {
        visibleRows[i]->FormRectangle(spanWidth, lineHeight);
        visibleGUI[i]->DefineRegion(spanWidth, lineHeight);
    }


    /*
    */
    //RefreshView();
}

void Dynacoe::DataGrid::AddRow() {
    rowCount++;
    Column * c;

    for(uint32_t i = 0; i < columns.size(); ++i) {
        c = &columns[i];
        c->rows.push_back("");
    }
    //RefreshView();
}


uint32_t Dynacoe::DataGrid::GetMaxViewPosition() {
    return rowCount - visibleRows.size();
}

uint32_t Dynacoe::DataGrid::GetViewPosition() {
    return viewIndex;
}



// Adds a new row if needed to support the view request

// deactivates the views current text and instead activates the proper
// text based on the current rows
void Dynacoe::DataGrid::RefreshView() {
    // clip to meaningful amount
    if (rowCount < visibleRows.size()) {
        viewIndex = 0;
    }
    else if (viewIndex + visibleRows.size() >= rowCount)
        viewIndex = GetMaxViewPosition();


    /*
    for(uint32_t i = 0; i < visibleText.size(); ++i) {
        RemoveComponent(visibleText[i]);
    }
    visibleText.clear();
    Column * col;
    for(uint32_t i = 0; i < columns.size(); ++i) {
        col = &columns[i];
        for(uint32_t n = viewIndex; n < col->rows.size() && n - viewIndex < visibleRows.size()-1; ++n) {
            col->rows[n]->node.local.position.y = (n-viewIndex+1)*(lineHeight);
            col->rows[n]->node.local.position.x = col->x;
            AddComponent(col->rows[n]);
            visibleText.push_back(col->rows[n]);
        }
    }
    */

    for(uint32_t n = 0; n < visibleRows.size(); ++n) {
        for(uint32_t i = 0; i < columns.size(); ++i) {
            visibleText[n][i]->text = "";
        }
    }
    for(uint32_t i = 0; i < columns.size(); ++i) {
        visibleText[0][i]->text = columns[i].title;
    }
    for(uint32_t n = viewIndex; n < rowCount && n - viewIndex < visibleRows.size()-1; ++n) {
        for(uint32_t i = 0; i < columns.size(); ++i) {
            visibleText[(n - viewIndex) + 1][i]->text = columns[i].rows[n];
        }
        if (n < tooltipText.size())
            visibleGUI[(n-viewIndex)+1]->SetTooltipText(tooltipText[n]);
    }


    wheelZone.DefineRegion(spanWidth, visibleRows.size()*lineHeight);

    // redefine scroll region
    //
    const int scroll_width_c = 4;
    int height = lineHeight*visibleRows.size();
    int unseenRows = (rowCount - visibleRows.size());
    int scrollHeight = height * (visibleRows.size() / (float) rowCount);

    if (visibleRows.size() < rowCount) {
        int pad = (height - scrollHeight) / 2;
        float position = viewIndex / (float) unseenRows;
        Vector scrollDim = {scroll_width_c, scrollHeight < 3 ? 3 : scrollHeight};
        scroll.node.local.position.x = spanWidth-scroll_width_c;
        scroll.node.local.position.y = (position * (pad*2));
        scroll.FormRectangle(scrollDim.x, scrollDim.y);
        //scroll.color.a = 1.f;

        // "true" scroll bar
        //scrollGrab.node.local.position.x = spanWidth-scroll_width_c*2;
        //scrollGrab.DefineRegion(scrollDim.x*2, height);
        scrollGrab.DefineRegion(spanWidth, visibleRows.size()*lineHeight);

    }

}

void Dynacoe::DataGrid::RemoveRow(uint32_t index) {
    if (index >= rowCount) return;
    std::vector<Text2D *> deleteme;
    for(uint32_t i = 0; i < columns.size(); ++i) {
        Column * col = &columns[i];
        col->rows.erase(col->rows.begin()+index);
    }
    rowCount--;
    //RefreshView();

}

void Dynacoe::DataGrid::Clear() {
    for(uint32_t i = 0; i < columns.size(); ++i) {
        Column * c = &columns[i];
        c->rows.clear();
    }
    rowCount = 0;
}

uint32_t Dynacoe::DataGrid::GetRowCount() {
    return rowCount;
}

std::string & Dynacoe::DataGrid::Get(uint32_t x, uint32_t y) {
    static std::string error;
    if (x >= columns.size()) return error;
    Column * c = &columns[x];
    if (y >= c->rows.size()) return error;

    return c->rows[y];
}


std::string & Dynacoe::DataGrid::GetTooltip(uint32_t y) {
    static std::string r;
    if (y >= rowCount) return r;
    while(tooltipText.size() <= y) {
        tooltipText.push_back("");
    }
    return tooltipText[y];
}

void Dynacoe::DataGrid::SetViewPosition(uint32_t i) {
    viewIndex = i;
    //RefreshView();
}

uint32_t Dynacoe::DataGrid::GetRowsVisible() {
    return visibleRows.size();
}


uint32_t Dynacoe::DataGrid::Width() {
    return spanWidth;
}

uint32_t Dynacoe::DataGrid::RowHeight() {
    return lineHeight;
}

DynacoeEvent(Dynacoe::DataGrid::begin_scroll_drag) {
    DataGrid * grid = self.IdentifyAs<DataGrid>();
    grid->isDragging = true;
    return true;
}



void Dynacoe::DataGrid::OnStep() {
    if (real_titleColor != titleColor) {
        real_titleColor = titleColor;
        for(uint32_t i = 0; i < columns.size(); ++i) {
            visibleText[0][i]->SetTextColor(titleColor);
        }
    }

    if (real_backgroundOddColor != backgroundOddColor ||
        real_backgroundEvenColor != backgroundEvenColor) {

        real_backgroundEvenColor = backgroundEvenColor;
        real_backgroundOddColor = backgroundOddColor;


        for(uint32_t i = 0; i < visibleRows.size(); ++i) {
            switch(i%2) {
              case 0: visibleRows[i]->color = backgroundEvenColor; break;
              case 1: visibleRows[i]->color = backgroundOddColor; break;
            }
        }


    }
    if (rowCount > visibleRows.size()) {
        if (wheelZone.IsHovered()) {
            if (Input::MouseWheel() < 0) {
                if (viewIndex < GetRowCount() - visibleRows.size())
                    SetViewPosition(viewIndex+1);
                printf("--\n");
            } else if (Input::MouseWheel() > 0) {
                if (viewIndex)
                    SetViewPosition(viewIndex-1);
                printf("++\n");

            }
            scrollAlpha = 1.f;
        } else {
            scrollAlpha = 0.f;

        }
    }


    if (isDragging) {
        if (!Input::GetState(MouseButtons::Left)) {
            isDragging = false;
            return;
        }

        int increase = Input::MouseYDelta()/3;

        // pixel -> viewrows
        float rate = .25*(rowCount - visibleRows.size())*(1.f / ((float)visibleRows.size()*lineHeight));
        if (rate >= 1) {
            increase *= rate;
        }

        // if possitive or would not cause underflow
        if (increase > 0 || (increase + viewIndex < increase)) {
            SetViewPosition(increase+viewIndex);
        }
        scrollAlpha = 1.f;
    }


    scroll.color.a = Mutator::StepTowards(scroll.color.a, scrollAlpha, .6, Mutator::Function::Quadratic);
    RefreshView();
}

void Dynacoe::DataGrid::SetRowsVisible(uint32_t rows) {
    while(visibleRows.size() > rows) {
        RemoveComponent(visibleRows[visibleRows.size()-1]);
        delete visibleRows[visibleRows.size()-1];
        visibleRows.erase(visibleRows.begin() + visibleRows.size()-1);

        for(uint32_t i = 0; i < columns.size(); ++i) {
            RemoveComponent(visibleText[visibleText.size()-1][i]);
            delete visibleText[visibleText.size()-1][i];
        }
        visibleText.erase(visibleText.begin() + visibleText.size()-1);
    }
    while(rows > visibleRows.size()) {
        Shape2D * rowBG = new Shape2D();
        rowBG->FormRectangle(spanWidth, lineHeight);
        rowBG->color = ((rowCount+1)%2) ? real_backgroundOddColor : real_backgroundEvenColor;
        rowBG->node.local.position = Vector(0, visibleRows.size()*lineHeight);

        GUI * gui = new GUI();
        gui->DefineRegion(spanWidth, lineHeight);
        gui->node.local.position = rowBG->node.local.position;
        // skip for title row
        if (visibleRows.size()) {
            gui->InstallHandler("on-click", master_click);
            gui->InstallHandler("on-enter", hover_enter);
            gui->InstallHandler("on-leave", hover_leave);
        }
        rowmap[gui] = visibleGUI.size();

        AddComponent(gui);
        AddComponent(rowBG);
        visibleRows.push_back(rowBG);
        visibleGUI.push_back(gui);

        Text2D * next;
        visibleText.push_back(std::vector<Text2D *>());
        for(uint32_t i = 0; i < columns.size(); ++i) {
            next = new Text2D;
            next->node.local.position.y = (visibleText.size()-1)*(lineHeight);
            next->node.local.position.x = columns[i].x;
            next->SetTextColor(columns[i].color);
            AddComponent(next);
            visibleText[visibleText.size()-1].push_back(next);
        }
    }
    //RefreshView();
}
