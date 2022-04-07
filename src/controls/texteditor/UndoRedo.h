#pragma once
#include "../../pch.h"

class Cmd
    {
    public:
        virtual void execute() = 0;
        virtual void undo() = 0;
        //stdstring dscr{ get; }
        virtual ~Cmd() {}
    };

class UndoRedo
    {
    private:
        std::stack<Cmd*> undoStack;
        std::stack<Cmd*> redoStack;
        void clear(std::stack<Cmd*>& stack);
    public:
        ~UndoRedo();
        bool CanUndo() { return undoStack.size()!= 0; }
        bool CanRedo() { return redoStack.size() != 0; }
        bool ignore = false;

        void undo();
        void redo();
        void add(Cmd* cmd);
        void execute(Cmd* cmd);
        void clear();
    };
