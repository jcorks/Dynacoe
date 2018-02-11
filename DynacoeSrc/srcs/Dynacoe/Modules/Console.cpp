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


#include <Dynacoe/Modules/Console.h>
#include <Dynacoe/Util/Chain.h>
#include <Dynacoe/Components/Text2D.h>
#include <Dynacoe/Components/Shape2D.h>
#include <Dynacoe/Modules/Graphics.h>
#include <Dynacoe/Modules/Input.h>
#include <Dynacoe/Components/Mutator.h>
#include <Dynacoe/Components/Object2D.h>
#include <Dynacoe/Util/Math.h>
#include <Dynacoe/Util/Random.h>
#include <Dynacoe/Dynacoe.h>
#include <Dynacoe/Interpreter.h>
#include <Dynacoe/Modules/Debugger.h>
#include <Dynacoe/Components/Node.h>
#include <Dynacoe/BuiltIn/DataGrid.h>
#include <cmath>
#include <cassert>


using namespace Dynacoe;

// Console::System << "System message" << Console::End;
// Console << "Normal message" << Console::End;
// Console::Info << "Info Message" << Console::End;

static DataGrid * mainGrid = nullptr;
static Entity * mainGridRoot = nullptr;

static bool console_overflow = false;
const uint32_t console_text_limit_line_c = 80;
Interpreter * Console::interp = nullptr;



class Dynacoe::DebugMessage : public Entity {
  public:

    void Set(const std::string & message, Shape2D * v, ConsoleStream::MessageType type){
        x = 0;
        y = 0;
        text.text = message;
        text.SetFontSize(13);
        text.node.local.position.x = 6;
        
        bg.FormRectangle(bg_width, bg_height);
        bg.color = (bg_color);
        

        AddComponent(&bg);
        AddComponent(&text);

        others.push_back(GetID());
        life.Clear(1.f);
        life.NewMutation(enter_life, 0.f, Mutator::Function::Logarithmic);
        life.NewMutation(enter_life + non_fade_life, 0.f, Mutator::Function::Constant);
        life.NewMutation(enter_life + non_fade_life + exit_life, 1.f, Mutator::Function::Quadratic);
        life.Start();


        tag.FormRectangle(5, bg_height);
        switch(type) {
            case ConsoleStream::MessageType::Normal:
                tag.color = (Color("white"));
                break;

            case ConsoleStream::MessageType::Warning:
                tag.color = (Color("orange"));

                break;

            case ConsoleStream::MessageType::Severe:
                tag.color = (Color("red"));
                break;

            case ConsoleStream::MessageType::Fatal:
                tag.color = (Color("dark red"));
                break;

            default:;
        }
        AddComponent(&tag);

        x = -bg_width;
        node.local.position.x = -bg_width + Graphics::GetCamera2D().node.local.position.x;

    }

    ~DebugMessage() {
        for(int i = 0; i < others.size(); ++i) {
            if (others[i] == GetID()) {
                others.erase(others.begin() + i);
            }
        }
    }




    void OnDraw(){


    }
    void OnStep(){
        if (life.Expired()) Remove();


        int index;
        for(int i = 0; i < others.size(); ++i) {
            if (others[i] == GetID()) {
                index = i;
                break;
            }
        }


        x = -bg_width * life.Value();
        y = Mutator::StepTowards((others.size()-index-1)*v_spacing_pixels, y, .5);
        node.local.position.x = Graphics::GetCamera2D().node.local.position.x + x;
        node.local.position.y = Graphics::GetCamera2D().node.local.position.y + y;
        
        if (y > Graphics::GetRenderCamera().Height()) {
            Remove();
        }

    }
  private:

    const int               v_spacing_pixels = 60;
    const float             enter_life = .3f;
    const float             non_fade_life = 3.f;
    const float             exit_life = .3f;
    const float             bg_width = 400.f;
    const float             bg_height = 50.f;
    const Dynacoe::Color    bg_color = Dynacoe::Color(32, 32, 32, 230);


    static std::vector<Entity::ID> others;
    Shape2D bg;
    Shape2D tag;
    

    
    Text2D text;
    Shape2D * v;

    Mutator life;
    float x;
    float y;


};


std::vector<Entity::ID> DebugMessage::others;



//uint32_t Console::LineModel::counter = 0;
Backend * Console::GetBackend() {
    return nullptr;
}





class ConsoleInputStream : public Entity {
  public:

    ConsoleInputStream() {
        inputStringAspect.SetTextColor("white");


        bg.color = "black";
        historyIter = 0;
        inputString = "";
        cursorStringAspect.FormRectangle(3, 10);
        cursorIter = 0;

        AddComponent(&bg);
        AddComponent(&inputStringAspect);
        AddComponent(&cursorStringAspect);

    }

    void OnStep() {
        int character;
        bool changed = false;
        static float saturation = 0.f;
        cursorStringAspect.color = (Color(255, 255, 255, 255*(.5*(1+sin(saturation)))));
        saturation += .04;
        node.local.position = Vector(0, Graphics::GetRenderCamera().Height() - 12);



        // Cursor control
        if (Input::IsPressed(Keyboard::Key_left)) {
            cursorIter--;
            saturation = 0.f;
        }

        if (Input::IsPressed(Keyboard::Key_right)) {
            cursorIter++;
            saturation = 0.f;
        }
        Math::Clamp(cursorIter, 0, inputString.size());


        // history control:
        // moves the old commadn to the active command slot
        if (Input::IsPressed(Keyboard::Key_up)) {
            historyIter--;
            changed = true;
            saturation = 0.f;
        }

        if (Input::IsPressed(Keyboard::Key_down)) {
            historyIter++;
            changed = true;
            saturation = 0.f;
        }

        if (changed) {
            Math::Clamp(historyIter, 0, history.size());
            if (historyIter == history.size()) {
                inputString = "";
                cursorIter = 0;
            } else {
                inputString = history[historyIter];
                cursorIter = inputString.size();
            }
        }



        // control input addition and deletion
        if (character = Input::GetLastUnicode()) {
            if (character == '\n') {
            } else if (character == '\b') {
                if (cursorIter > 0) {
                    saturation = 0.f;
                    inputString =
                        inputString.substr(0, cursorIter-1) +

                        inputString.substr(cursorIter, std::string::npos);
                    cursorIter--;
                }
            } else {
                saturation = 0.f;
                inputString =
                    inputString.substr(0, cursorIter) +
                    (char)character +
                    inputString.substr(cursorIter, std::string::npos);

                cursorIter++;
            }
            changed = true;
        }
        Math::Clamp(cursorIter, 0, inputString.size());


        // update displayed string
        if (changed) {
            inputStringAspect.text = Chain() << "$  " << inputString;
        }
    }

    std::string Consume() {
        std::string out = inputString;
        if (out.length())
            history.push_back(inputString);
        historyIter = history.size();
        inputString = "";
        inputStringAspect.text = ("$  ");
        return out;
    }

    void OnDraw() {
        bg.FormRectangle(
            Graphics::GetRenderCamera().Width(),
            12
        );
        
        cursorStringAspect.node.local.position = (inputStringAspect.GetCharPosition(3+cursorIter));

    }

  private:
    std::string  inputString;
    Text2D   inputStringAspect;
    Shape2D  cursorStringAspect;
    int cursorIter;        


    Shape2D  bg;
    std::vector<std::string> history;
    int historyIter;
};



const Color system_color_c          (128, 255, 255, 255);
const Color info_color_c            (255, 255, 128, 255);
const Color error_color_c           (255, 64,  64,  255);
const Color warn_color_c            (255, 128, 64,  255);

Entity *                                    Console::messages = nullptr;

bool                 Console::shown = false;
bool                 Console::locked = 0;
Console::MessageMode Console::messageMode;
int                  Console::fontHeight = 0;
float                Console::basePositionOffsetRatio = -1.f;
Shape2D *        Console::base = nullptr;
ConsoleInputStream * Console::streamIn = nullptr;
bool                 Console::inputActive = false;
uint32_t             Console::viewOffsetY = 0;

std::vector<std::pair<std::string, ConsoleStream::MessageType>> Console::stream;
ConsoleStream::MessageType ConsoleStream::type;

const char * Console::End = "\n";





Console::Console() {

}


void Console::Init() {


    locked = false;
    shown  = false;
    messageMode = MessageMode::Standard;
    fontHeight = 10;

    //console_main_font = Assets::Load("otf", "console.otf");

    streamIn = Entity::Create<ConsoleInputStream>().IdentifyAs<ConsoleInputStream>();
    messages = Entity::Create<Entity>().Identify();
    interp = new Interpreter;
    viewOffsetY = 0;
    
    mainGridRoot = Entity::Create<Entity>().Identify();

    
    mainGrid = Entity::Create<DataGrid>().IdentifyAs<DataGrid>();
    mainGridRoot->Attach(mainGrid->GetID());
    mainGridRoot->node.local.rotation.x = 180;
    mainGridRoot->node.local.rotation.y = 180;

    mainGrid->backgroundEvenColor = {.0f, .0f, .0f, .8};
    mainGrid->backgroundOddColor  = {.0f, .0f, .0f, .8};

    Engine::AttachManager(mainGridRoot->GetID());
    Entity * messageRoot = Entity::CreateReference<Entity>();
    messageRoot->Attach(messages->GetID());
    Engine::AttachManager(messages->GetID());
    mainGrid->Attach(streamIn->GetID());
    mainGrid->AddColumn("_Console", 1, {.8f, 1.f, .9f, 1.f});
}

void Console::InitAfter() {
    AddDefaultCommands();
}
void Console::RunBefore() {
    static float saturator = 0;
    static float lastW, lastH;
    if (lastW != Graphics::GetRenderCamera().Width() ||
        lastH != Graphics::GetRenderCamera().Height()) {

        mainGrid->SetColumnWidth(0, Graphics::GetRenderCamera().Width());
        mainGrid->SetRowsVisible((Graphics::GetRenderCamera().Height())/12 - 1);

        lastW = Graphics::GetRenderCamera().Width();
        lastH = Graphics::GetRenderCamera().Height();
    }
    /*
    if (inputActive && shown) {
        streamIn->Step();
    }
    */



    if (Input::MouseWheel() == -1 || Input::GetState(Keyboard::Key_pageDown)) {
        //if (saturator < 0) saturator = 0;
        saturator += .5f;
    }


    if (Input::MouseWheel() == 1 || Input::GetState(Keyboard::Key_pageUp)) {
        //if (saturator > 0) saturator = 0;
        saturator -= .5f;
    }


    viewOffsetY += (int)saturator;
    saturator *= .83;

}
void Console::RunAfter()  {
    ProcessStreamOutput();


    if ((Input::IsHeld(Keyboard::Key_lshift) ||
         Input::IsHeld(Keyboard::Key_rshift)) &&
        Input::IsPressed(Keyboard::Key_tab)) 
        shown = !shown;



    // interactive console
    
    if (shown) {

            
        
        
        if (Input::IsPressed(Keyboard::Key_enter)) {
            inputActive = !inputActive;

            // halted input
            if (!inputActive) {
                std::string inputString = streamIn->Consume();
                ConsoleStream(AcquireStreamOutput) << ConsoleStream::MessageType::Normal <<  ">" << inputString << "\n";
                if (!inputString.empty()) {
                    Console::Info() << interp->RunCommand(inputString) << '\n';
                }
            }
        }

    }
    
}
void Console::DrawBefore(){}

void Console::OverlayMessageMode(MessageMode m) {
    ProcessStreamOutput();
    messageMode = m;
}

Console::MessageMode Console::GetOverlayMessageMode() {
    return messageMode;
}

ConsoleStream::ConsoleStream(FinishedCallback cb) {
    finish = cb;
    str = "";
}

ConsoleStream::~ConsoleStream() {
    if (finish) {
        finish(str, type);
    }
}


ConsoleStream::ConsoleStream(const ConsoleStream & other) {
    *this = other;
}

ConsoleStream & ConsoleStream::operator=(const ConsoleStream & other) {
    str = "";
    finish = other.finish;
    type = other.type;
    return *this;
}



ConsoleStream ConsoleStream::operator<<(const Chain & s) {
    ConsoleStream out(finish);
    out.str = str+s.ToString();
    str = "";

    return out;
}


ConsoleStream  ConsoleStream::operator<<(MessageType t) {
    type = t;
    return *this;
}







ConsoleStream  Console::System() {
    //return *this << system_color_c;
    return ConsoleStream(AcquireStreamOutput) << ConsoleStream::MessageType::Normal;
}
ConsoleStream  Console::Info() {
    //return *this << info_color_c;
    return ConsoleStream(AcquireStreamOutput) << ConsoleStream::MessageType::Normal;
}
// TODO: decide whether to show consoles on errors.
ConsoleStream  Console::Error() {
    //return *this << error_color_c;
    return ConsoleStream(AcquireStreamOutput) << ConsoleStream::MessageType::Fatal;
}
ConsoleStream  Console::Warning() {
    //return *this << warn_color_c;
    return ConsoleStream(AcquireStreamOutput) << ConsoleStream::MessageType::Warning;
}


bool Console::IsVisible() {
    return shown;
}

void Console::Show(bool b) {
    shown = b;
}



bool Console::IsLocked() {
    return locked;
}

void Console::Lock(bool b) {
    locked = b;
}





uint32_t Console::GetNumLines() {
    return mainGrid->GetRowCount();;
}


std::string Console::GetLine(uint32_t i) {
    // clamp to valid range, no errors
    if (i < 0) i = 0;
    if (i >= GetNumLines()) i = GetNumLines()-1;

    return "";
    return mainGrid->Get(0, i);
}


void Console::Clear() {
    mainGrid->Clear();
}







// Idea 1:

// anytime a line slot changes: recopmile corresponding line
// a line slot changes if: the line object owned by that line slot was altered text-wise

void Console::DrawAfter() {
    messages->step = messages->GetNumChildren();
    messages->draw = messages->GetNumChildren();


    
    // ease position
    basePositionOffsetRatio =
        Mutator::StepTowards(
            basePositionOffsetRatio,
            (shown ? 0.f : -1.f), .14
        );
    /*
    if (basePositionOffsetRatio < -.99f) {
        mainGridRoot->draw = false;
        return;
    } else {
        mainGridRoot->node.local.position = Graphics::GetCamera2D().node.local.position 
                    + Vector(basePositionOffsetRatio * Graphics::GetRenderCamera().Width(), 0);

        mainGridRoot->draw = true;
        
    }
    */


    if (basePositionOffsetRatio < -.99f) {
        mainGridRoot->draw = false;
        return;
    } else {
        mainGridRoot->node.local.rotation.x = basePositionOffsetRatio*180;
        mainGridRoot->node.local.rotation.y = basePositionOffsetRatio*180;
        mainGridRoot->node.local.position = Graphics::GetCamera2D().node.local.position;
        mainGridRoot->draw = true;
        
    }
    
    
    
    
    /*
    // not proud of this, but...
    int viewOffsetY_tmp = viewOffsetY;
    Math::Clamp(viewOffsetY_tmp, 0, lines.size() - lineViews.size());
    viewOffsetY = viewOffsetY_tmp;


    // todo
    //AdjustViewSize();





    Vector pos = Graphics::GetFlatPosition();
    pos.x += basePositionOffsetRatio * Graphics::GetRenderResolutionWidth();

    base->SetPosition(pos);
    Graphics::Draw(*base);

    pos = Vector();
    for(uint32_t i = 0; i < lineViews.size(); ++i) {
        lineViews[i]->OnDraw(pos);
        pos.y += fontHeight;
    }
    */

    /*
    if (inputActive) {
        streamIn->Draw();
    }
    */
    streamIn->draw = inputActive;
    streamIn->step = inputActive;
    

}











/// privates

// Make sure that the line views are connected to proper lines
// and that we are even showing the proper lineviews
/*
void Console::AdjustViewSize() {
    if (!base) {
        base = new Shape2D;
        base->color = (Dynacoe::Color(14, 14, 14, 196));
    }

    base->FormRectangle(
        Graphics::GetRenderResolutionWidth(),
        Graphics::GetRenderResolutionHeight()
    );
    int i = 0;

    for(;
            i < lines.size() &&
            (i+3)*fontHeight < Graphics::GetRenderResolutionHeight()
        ; ++i) {


        while (i >= lineViews.size()) {
            lineViews.push_back(new LineView);
        }

        if (viewOffsetY+i < lines.size())
            lineViews[i]->Connect(lines[i+viewOffsetY]);
    }
    console_overflow = (i+3)*fontHeight >= Graphics::GetRenderResolutionHeight();
}
*/

// process expired ConsoleStream
void Console::AcquireStreamOutput(const std::string & str, ConsoleStream::MessageType type) {
    // last stream type overrides previous, so the actual used color in the line model is from the
    // streamType at the type a \n is inserted.
    std::cout << str;
    stream.push_back(
        std::pair<std::string, ConsoleStream::MessageType>(str, type)
    );

}

void Console::ProcessStreamOutput() {
    for(uint32_t i = 0; i < stream.size(); ++i) {
        ProcessStreamIteration(stream[i].first, stream[i].second);
    }
    stream.clear();
}


void Console::ProcessStreamIteration(const std::string & strSrc, ConsoleStream::MessageType message) {
    std::string str = strSrc;

    // add newlines to lines of text that are too large to fit
    // process everythign but the last limt characters

    /*


    // If no actual string, just update the color
    if (str.empty() && lines.size()) {
        lines[lines.size()-1]->Add("", message);
        return;
    }
    int markerIter = 0;
    */
    
    //if (!lines.size()) lines.push_back(new LineModel);
    if (mainGrid->GetRowCount() == 0) mainGrid->AddRow();

    // its valid to have no string, but a color directive

    int newLineIndex = 0;
    int originalIndx = mainGrid->GetRowCount()-1;
    bool console_overflow = (mainGrid->GetViewPosition() == mainGrid->GetMaxViewPosition());

    for(int i = 0; i < str.size(); ++i) {
        if (mainGrid->Get(0, originalIndx+newLineIndex).size()> console_text_limit_line_c) {
            str.insert(i, "\n");
        }
        if (str[i] != '\n') {
            mainGrid->Get(0, originalIndx+newLineIndex) += (Chain() << str[i]); //message);
        } else {
            mainGrid->AddRow();
            // break up input with multiple lines into separate message calls
            if (!shown)
                PostMessageConsole(mainGrid->Get(0, originalIndx+newLineIndex), message);

            //if (lines.size()*fontHeight > Graphics::GetRenderResolutionHeight())            
            if (console_overflow)
                mainGrid->SetViewPosition(mainGrid->GetRowCount()-1);

            newLineIndex++;
        }
    }
    


}


void Console::PostMessageConsole(const std::string & str, ConsoleStream::MessageType type) {
    const int message_v_spacing = 40;


    if (messageMode == MessageMode::Standard) {
        DebugMessage * m = Entity::CreateReference<DebugMessage>();
        m->Set(
            str,
            NULL,
            type
        );
        messages->Attach(
            m->GetID()
        );
    }

}





//////////////////////////
//////////////////////////
/// Console commands

static const char * ask_answers[] = {
    "Very much so.",
    "Not really.",
    "Not in the slightest.",
    "Without a doubt.",
    "Quite possibly.",
    "Probably not.",
    "Yeah...right...",
    "Maybe at some point.",
    "Unlikely",
    "You wish.",
    "Excuse me?",
    "I don't have time for this.",
    "Most definitely.",
    "Most definitely not.",
    "Believe it or not, yeah.",
    "Absolutely not."
};

class Command_Ask : public Interpreter::Command {
  public:


    std::string operator()(const std::vector<std::string> & argvec) {
        int val = Random::Value()*(sizeof(ask_answers)/(sizeof(char*)));
        return std::string(ask_answers[val]) +'\n';
    }

    std::string Help() const {
        return "A *very* helpful yes/no answerer. Ask your most devious yes/no questions and be amazed!";
    }

};


class Command_Print : public Interpreter::Command {
    std::string operator()(const std::vector<std::string> & argvec) {

        for(int i = 1; i < argvec.size(); ++i) {
            Console::Info() << argvec[i] << " ";
        }
        return "";
    }

    std::string Help() const {
        return "Prints the arguments to the console.";
    }
};


class Command_Exit : public Interpreter::Command {
  public:

    std::string operator()(const std::vector<std::string> & argvec) {
        Engine::Quit();
        return "Bye bye!";
    }
    std::string Help() const {
        return "Exits the main Engine loop.";
    }

};


class Command_ViewID : public Interpreter::Command {
  public:

    std::string operator()(const std::vector<std::string> & argvec) {
        if (argvec.size() != 2) return "Usage: view-id idnumber";

        uint64_t id = atoi(argvec[1].c_str());
        Debugger::ViewID(id);

        return "";
    }
    std::string Help() const {
        return "Opens a new Debugger window open with the Entity refered to by the ID";
    }

};



class EntityModCommand : public Interpreter::Command {

  public:


    std::string operator()(const std::vector<std::string> & argvec) {
        if (argvec.size() == 1 || argvec.size() > 4) {
            Console::Info() << Help();
            return "";
        }

        std::string ent;
        std::string property;
        std::string newValue;

        ParseArgs(argvec, ent, property, newValue);




        uint32_t id;
        if (sscanf(ent.c_str(), "[%u]", &id)) {

            if (!property.size()) {
                return std::string("Missing property for object ID ") + ent;
            }


            Entity * e = Entity::ID(id).Identify();
            if (!e) {
                Console::Info() << "No entity with ID " << (int) id << "\n";
                return "";
            }


            Variable var = e->GetWatched(property);


            if (newValue.size()) {
                var.Set(newValue);
            }


            // print current state. If value was set, it will reflect the changes
            if (var.GetName().size()) {
                return var.ToString();
            } else {
                Console::Info() << "No such variable " << property << "\n";
                return "";
            }


        } else {
            // for each matched name, rerun command with their ID
            Entity * w;
            std::vector<Entity::ID> ents;

            if (Engine::Root().Valid())
                ents = Engine::Root().Identify()->FindChildByName(ent);

            if (!ents.size()) {
                Console::Info() << "No entities with name " << ent << "\n";
                return "";
            }

            std::string out = "";
            // If no property, just print the IDs of all found ents
            if (!property.size()) {
                for(int i = 0; i < ents.size(); ++i) {
                    out += (Chain() << "[" << (int)ents[i].Value() << "]\n");
                }
                return out;
            }


            std::vector<std::string> argSub(argvec.begin(), argvec.end());

            for(int i = 0; i < ents.size(); ++i) {
                argSub[1] = (Chain() << "[" << (int)ents[i].Value() << "]." << property);
                out += (*this)(argSub) + '\n';
            }

            return out;
        }


        return "";
    }

    std::string Help()const {
        return
            "Modifies and displays the value of one or more entity debug properties.\n"
            "Properties are set using the Watch* functions in the Entity interface\n\n"
            "usage:\n"
            "    mod object.property [= \"value\"]\n"
            "\n\n"
            "The object query is mandatory and can be designated in two ways: \n"
            "   - A unique ID number in the following format: \"[#]\".\n"
            "   - An entity name. If this refers to more than one instance of an Entity,\n"
            "     every entity for which it is a match is specified, the mod command will be applied\n"
            "     as if the command was run with each entity's ID.\n"
            "If a value is not included, the property is simply printed. If\n"
            "the value is compatible with he type of the variable, it will\n"
            "update the variable, else no action is taken. In both situations\n"
            "the current value of the variable will be printed .\n";

    }


  private:

    void ParseArgs(
            const std::vector<std::string> & argvec,
            std::string & ent,
            std::string & property,
            std::string & newValue) {


        Chain target = argvec[1];
        target.SetDelimiters(".");
        ent = target.GetLink();
        target++;
        uint32_t lpos = target.GetLinkPos();
        property = target.ToString().substr(target.GetLinkPos(), target.ToString().size());

        if (argvec.size() < 4) return;
        if (argvec[2] != "=") return;

        newValue = argvec[3];

    }


};



void Console::AddCommand(const std::string & str, Interpreter::Command * cmd) {
    interp->AddCommand(str, cmd);
}




// some built-in commands for the console
void Console::AddDefaultCommands() {
    interp->AddCommand("ask",     new Command_Ask);
    interp->AddCommand("exit",    new Command_Exit);
    interp->AddCommand("quit",    new Command_Exit);
    interp->AddCommand("seeya",   new Command_Exit);
    interp->AddCommand("print",   new Command_Print);
    interp->AddCommand("mod",     new EntityModCommand);
    interp->AddCommand("view-id", new Command_ViewID);
    //interp->AddCommand("backend", new Command_BackendExt);

}
