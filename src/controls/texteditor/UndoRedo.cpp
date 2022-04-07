#include "UndoRedo.h"

void UndoRedo::clear(std::stack<Cmd*>& stack)
    {
    while (!stack.empty())
        {
        auto cmd = stack.top();
        stack.pop();
        delete cmd;
        }
    }

UndoRedo::~UndoRedo()
    {
    clear(undoStack);
    clear(redoStack);
    }

void UndoRedo::undo()
    {
    if (undoStack.empty())
        return;
    auto cmd = undoStack.top();
    undoStack.pop();
    ignore = true;
    cmd->undo();
    ignore = false;
    redoStack.push(cmd);
    }

void UndoRedo::redo()
    {
    if (redoStack.empty())
        return;
    auto cmd = redoStack.top();
    redoStack.pop();
    ignore = true;
    cmd->execute();
    ignore = false;
    undoStack.push(cmd);
    }

void UndoRedo::add(Cmd* cmd)
    {
    if (ignore)
        {
        delete cmd;
        return;
        }

    undoStack.push(cmd);
    clear(redoStack); 
    }

/// <summary>
/// Add and immediately execute command
/// </summary>
/// <param name="cmd"></param>
void UndoRedo::execute(Cmd* cmd)
    { 
    if (ignore)
        return;
    bool oldIgnore = ignore;
    ignore = true;
    cmd->execute();
    ignore = oldIgnore;
    add(cmd);
    }

void UndoRedo::clear()
    {
    clear(undoStack);
    clear(redoStack);
    }
