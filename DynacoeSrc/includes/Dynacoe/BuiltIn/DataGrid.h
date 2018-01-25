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

#ifndef DC_DATA_GRID_INCLUDED
#define DC_DATA_GRID_INCLUDED

#include <Dynacoe/Entity.h>
#include <Dynacoe/Components/Text2D.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Components/GUI.h>
#include <Dynacoe/Color.h>

namespace Dynacoe {
    
/// \brief Built-in Entity for drawing a grid of values.
///
class DataGrid : public Entity {
  public:
    
    /// \brief Background color for even-count cells
    ///
    Color backgroundEvenColor;
    
    /// \brief Background color for odd-count cells 
    ///
    Color backgroundOddColor;
    
    /// \brief Color for the title text 
    ///
    Color titleColor;
    
    
    DataGrid();
    ~DataGrid(){}
    
    /// \brief Adds a new column with a title, a specified width, and an option color 
    /// for the text.
    ///
    void AddColumn(const std::string & title, int width, const Color & color = Color("#EFEFEF"));

    /// \brief Sets the width in pixels for the specified column.
    ///
    void SetColumnWidth(uint32_t column, uint32_t width);
    
    /// \brief Callback called when a cell is clicked.
    ///
    /// The first member of args is the row clicked as an integer.
    Component::EventHandler clickCallback;

    /// \brief Appends a new row to the grid
    //
    void AddRow();
    
    /// \brief removes the specified row by index.
    ///
    void RemoveRow(uint32_t);
    
    /// \brief  Will only draw display up to the given number of rows
    ///
    void SetRowsVisible(uint32_t);
        
        
    /// \brief Returns the number of rows
    ///
    uint32_t GetRowCount();

    /// \brief Clears all rows.
    ///
    void Clear();

    /// \brief Returns the string @ the column and row given 
    ///
    std::string & Get(uint32_t x, uint32_t y);

    /// \brief Returns the tooltip string @ the row given 
    ///
    /// Tooltips are messages that hover over the mouse 
    /// when the pointer is at the specific row.
    std::string & GetTooltip(uint32_t y);

    /// \brief Sets the initial viewing row 
    ///
    void SetViewPosition(uint32_t i);

    /// \brief Returns the current viewing index at the top. 
    /// 0 is the default (aka the first row)
    ///
    uint32_t GetViewPosition();

    /// \brief Returns the maximum currently allowable viewing position 
    /// based on the current DataGrid parameters.
    ///
    uint32_t GetMaxViewPosition();

    /// \brief Returns the currently viewable number of rows at a time.
    ///
    uint32_t GetRowsVisible();

    /// \brief Returns the row in pixels.
    ///
    uint32_t Width();
    
    /// \brief Returns the height of a row.
    ///
    uint32_t RowHeight();
    
  private:
    
    struct Column {
        std::string title;
        int x;
        int width;
        Color color;
        std::vector<std::string> rows;
    };
    
    uint32_t spanWidth;
    
    std::vector<Column> columns;
    std::vector<Shape2D *> visibleRows;
    std::vector<GUI *> visibleGUI;
    std::vector<std::vector<Text2D  *>> visibleText;
    std::vector<std::string> tooltipText;
    Color real_backgroundEvenColor;
    Color real_backgroundOddColor;
    Color real_titleColor;
    Color real_defaultTextColor;
    
    GUI wheelZone;
    Shape2D scroll;
    GUI scrollGrab;
    
    uint32_t rowCount;
    uint32_t viewIndex;
    
    const int lineHeight = 12;
    Text2D error;


    void OnStep();
    void AddNeededView();
    void RefreshView();
    
    float scrollAlpha;
    bool isDragging;
    
    std::map<GUI *, uint32_t> rowmap;
    static DynacoeEvent(master_click);
    static DynacoeEvent(hover_enter);
    static DynacoeEvent(hover_leave);
    static DynacoeEvent(begin_scroll_drag);
};
}

#endif