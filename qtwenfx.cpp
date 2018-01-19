#include <QApplication>
#include <QMainWindow>
#include <QSettings>
#include <QShortcut>
#include <QWebEngineView>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineSettings>

class WebPlayer : public QMainWindow
{
    Q_OBJECT

public:
    explicit WebPlayer(QWidget *parent = nullptr);

private slots:
    void slotFullScreenRequested(QWebEngineFullScreenRequest request);
    void slotKeyQuit();
    void slotPageLoaded();

private:
    QWebEngineView *m_view;
    QWebEngineView *m_viewBg;
    bool m_wasMaximized;
    QSettings m_appSettings;
    QShortcut m_keyQuit;

    void saveWindowState();
    void restoreWindowState();

protected:
    void closeEvent(QCloseEvent *);
};


WebPlayer::WebPlayer(QWidget *parent)
    : QMainWindow(parent),
    m_view(new QWebEngineView(this)),
    m_viewBg(new QWebEngineView(this)),
    m_appSettings("qtwenfx","WindowState", this),
    m_keyQuit(this)
{

    restoreWindowState();

    m_keyQuit.setKey(Qt::Key_Escape);
    connect(&m_keyQuit, SIGNAL(activated()), this, SLOT(slotKeyQuit()));

    m_wasMaximized = this->isMaximized();

    m_viewBg->page()->setBackgroundColor(Qt::black);
    m_viewBg->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_viewBg->hide();

    m_view->page()->setBackgroundColor(Qt::black);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);


    connect(m_viewBg->page(),
            &QWebEnginePage::fullScreenRequested,
            this,
            &WebPlayer::slotFullScreenRequested);
    connect(m_viewBg, SIGNAL(loadFinished(bool)), SLOT(slotPageLoaded()));
    m_viewBg->load(QUrl(QStringLiteral("https://www.netflix.com")));

    setCentralWidget(m_view);

    m_view->setHtml(QString("<html>" \
                    "<head><style>html, body, #wrapper { height: 100%; width: 100%; margin: 0; padding: 0; border: 0;}" \
                    "#wrapper td { vertical-align: middle; text-align: center; } </style></head>" \
                    "<body><table id='wrapper'><tr><td><img src='qrc:/netflix_logo.svg' style='max-height:30%; max-width:30%'; alt="" /></td></tr></table>" \
                    "</body></html>"));
}

void WebPlayer::slotFullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        if (this->isFullScreen())
            return;
        request.accept();
        m_wasMaximized = this->isMaximized();

        this->showFullScreen();
    } else {
        if (!this->isFullScreen())
            return;

        request.accept();
        if (m_wasMaximized) {
            this->showNormal();
            this->showMaximized();
        } else {
            this->showNormal();
        }
    }
}

void WebPlayer::saveWindowState()
{
    if (this->isFullScreen())
        this->showNormal();
    m_appSettings.setValue("state/mainWindowState", saveState());
    m_appSettings.setValue("geometry/mainWindowGeometry", saveGeometry());

}

void WebPlayer::restoreWindowState()
{
    restoreState(m_appSettings.value("state/mainWindowState").toByteArray());
    restoreGeometry(m_appSettings.value("geometry/mainWindowGeometry").toByteArray());
}

void WebPlayer::closeEvent(QCloseEvent *)
{
    saveWindowState();
}

void WebPlayer::slotKeyQuit()
{
    saveWindowState();
    QApplication::quit();
}

void WebPlayer::slotPageLoaded()
{
    m_view->setPage(m_viewBg->page());
    setWindowTitle(m_view->title());
}

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setApplicationDisplayName("");
    WebPlayer wp;
    wp.show();
    return app.exec();
}

#include "qtwenfx.moc"
