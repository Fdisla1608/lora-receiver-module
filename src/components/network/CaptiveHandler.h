class CaptiveRequestHandler : public AsyncWebHandler
{
private:
public:
    const char *index PROGMEM;
    CaptiveRequestHandler();

    virtual ~CaptiveRequestHandler()
    {
    }

    bool canHandle(AsyncWebServerRequest *request)
    {
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", index);
    }
};

CaptiveRequestHandler::CaptiveRequestHandler()
{
    index = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8" />
            <meta name="viewport" content="width=device-width, initial-scale=1.0" />
            <title>Document</title>
        <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            color: antiquewhite;
            font-family: Arial, Helvetica, sans-serif;
            background: RGB(25, 135, 84);
        }
        .welcome-container {
            display: flex;
            justify-content: space-between;
            flex-direction: column;
            width: 40vh;
            height: 20vh;
            background-color: white;
            border: 2px rgb(8, 8, 8) solid;
            border-radius: 10px;
            padding: 20px;
        }
        .welcome-title {
            width: 100%;
            text-align: center;
            font-size: 4.5vh;
            color: black;
        }

        .welcome-button--box {
            display: flex;
            justify-content: center;
        }

        .welcome-button {
            display: flex;
            justify-content: center;
            align-items: center;
            text-decoration: none;
            width: 30vw;
            height: 5vh;
            border: none;
            font-size: 2rem;
            color: white;
            background-color: #001177;
            border-radius: 5px;
        }

        .welcome-button:hover {
            color: RGB(25, 135, 84);
            background-color: white;
            border: black 1px solid;
        }
        </style>
        </head>
        <body>
            <div class="welcome-container">
            <div class="welcome-title">Trumore Module Configuration</div>
            <div class="welcome-button--box">
                <a href="/network" class="welcome-button">Next</a>
            </div>
            </div>
        </body>
        </html>
)rawliteral";
}
