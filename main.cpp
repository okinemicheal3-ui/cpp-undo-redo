#include <iostream>
#include <string>
#include <stack>
#include "httplib.h" // We will download this library

using namespace std;

// --- Your Original Undo/Redo Logic ---
enum ActionType { TYPE_TEXT, DELETE_TEXT };
struct Action { ActionType type; string text; int pos; };

class TextEditor {
private:
    string document;
    stack<Action> undoStack;
    stack<Action> redoStack;
    void clearRedo() { while (!redoStack.empty()) redoStack.pop(); }

public:
    void type(const string& text) {
        if (text.empty()) return;
        document += text;
        undoStack.push({TYPE_TEXT, text, (int)document.size() - (int)text.size()});
        clearRedo();
    }
    void undo() {
        if (undoStack.empty()) return;
        Action act = undoStack.top(); undoStack.pop();
        if (act.type == TYPE_TEXT) document.erase(act.pos, act.text.size());
        else document.insert(act.pos, act.text);
        redoStack.push(act);
    }
    void redo() {
        if (redoStack.empty()) return;
        Action act = redoStack.top(); redoStack.pop();
        if (act.type == TYPE_TEXT) document.insert(act.pos, act.text);
        else document.erase(act.pos, act.text.size());
        undoStack.push(act);
    }
    string getDoc() const { return document; }
};

// Global instance (Note: For a real app, use sessions, but this is fine for a demo)
TextEditor editor;

int main() {
    httplib::Server svr;

    // 1. Serve the HTML Frontend
    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        string html = R"(
            <!DOCTYPE html>
            <html>
            <head>
                <title>C++ Undo/Redo Deployed</title>
                <style>
                    body { font-family: sans-serif; max-width: 600px; margin: 40px auto; }
                    #doc { border: 2px dashed #333; padding: 20px; font-size: 20px; min-height: 40px; margin-bottom: 20px; background: #f9f9f9;}
                    button { padding: 10px 15px; margin-right: 10px; cursor: pointer; }
                </style>
            </head>
            <body>
                <h2>C++ Stack Application: Undo/Redo</h2>
                <div id="doc">Empty Document</div>
                <input type="text" id="txt" placeholder="Enter text...">
                <button onclick="send('/type?text=' + document.getElementById('txt').value)">Type</button>
                <button onclick="send('/undo')">Undo</button>
                <button onclick="send('/redo')">Redo</button>
                <script>
                    function send(endpoint) {
                        fetch(endpoint)
                            .then(res => res.text())
                            .then(data => { document.getElementById('doc').innerText = data; });
                    }
                </script>
            </body>
            </html>
        )";
        res.set_content(html, "text/html");
    });

    // 2. API Endpoints for Stack Operations
    svr.Get("/type", [](const httplib::Request &req, httplib::Response &res) {
        if (req.has_param("text")) editor.type(req.get_param_value("text"));
        res.set_content(editor.getDoc(), "text/plain");
    });

    svr.Get("/undo", [](const httplib::Request &, httplib::Response &res) {
        editor.undo();
        res.set_content(editor.getDoc(), "text/plain");
    });

    svr.Get("/redo", [](const httplib::Request &, httplib::Response &res) {
        editor.redo();
        res.set_content(editor.getDoc(), "text/plain");
    });

    cout << "Server starting on port 8080..." << endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}