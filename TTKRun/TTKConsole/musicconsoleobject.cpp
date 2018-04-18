#include "musicconsoleobject.h"
#include "musicplayer.h"
#include "musicplayedlist.h"
#include "musiccoreutils.h"
#include "musicformats.h"
#include "musiclistconfigmanager.h"
#include "musichotkeymanager.h"
#include "musictime.h"

#ifdef MUSIC_WINEXTRAS
#include <QCommandLineOption>
#include <QCommandLineParser>
#endif

MusicConsoleObject::MusicConsoleObject(QObject *parent)
    : QObject(parent)
{
    m_musicPlayer = new MusicPlayer(this);
    m_musicPlayList = new MusicPlayedlist(this);

    m_volume = 100;
    m_playbackMode = "Order";
    m_enhanced = "Off";

    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayOrder);
    m_musicPlayer->setPlaylist(m_musicPlayList);

    connect(m_musicPlayer, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(m_musicPlayer, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(m_musicPlayList, SIGNAL(currentIndexChanged(int)), SLOT(currentIndexChanged(int)));

    M_HOTKEY_PTR->addHotKey("Ctrl+B");
    M_HOTKEY_PTR->addHotKey("Ctrl+Left");
    M_HOTKEY_PTR->addHotKey("Ctrl+Right");
    M_HOTKEY_PTR->addHotKey("Ctrl+Up");
    M_HOTKEY_PTR->addHotKey("Ctrl+Down");
    M_HOTKEY_PTR->addHotKey("Ctrl+1");
    M_HOTKEY_PTR->addHotKey("Ctrl+2");
    M_HOTKEY_PTR->addHotKey("Ctrl+3");
    M_HOTKEY_PTR->addHotKey("Ctrl+4");
    M_HOTKEY_PTR->addHotKey("Ctrl+5");
    M_HOTKEY_PTR->addHotKey("Alt+1");
    M_HOTKEY_PTR->addHotKey("Alt+2");
    M_HOTKEY_PTR->addHotKey("Alt+3");
    M_HOTKEY_PTR->addHotKey("Alt+4");
    M_HOTKEY_PTR->addHotKey("Alt+5");
    M_HOTKEY_PTR->addHotKey("Ctrl+Q");

    connect(M_HOTKEY_PTR->getHotKey(0), SIGNAL(activated()), SLOT(musicStatePlay()));
    connect(M_HOTKEY_PTR->getHotKey(1), SIGNAL(activated()), SLOT(musicPlayPrevious()));
    connect(M_HOTKEY_PTR->getHotKey(2), SIGNAL(activated()), SLOT(musicPlayNext()));
    connect(M_HOTKEY_PTR->getHotKey(3), SIGNAL(activated()), SLOT(musicActionVolumePlus()));
    connect(M_HOTKEY_PTR->getHotKey(4), SIGNAL(activated()), SLOT(musicActionVolumeSub()));

    connect(M_HOTKEY_PTR->getHotKey(5), SIGNAL(activated()), SLOT(musicPlayOrder()));
    connect(M_HOTKEY_PTR->getHotKey(6), SIGNAL(activated()), SLOT(musicPlayRandom()));
    connect(M_HOTKEY_PTR->getHotKey(7), SIGNAL(activated()), SLOT(musicPlayListLoop()));
    connect(M_HOTKEY_PTR->getHotKey(8), SIGNAL(activated()), SLOT(musicPlayOneLoop()));
    connect(M_HOTKEY_PTR->getHotKey(9), SIGNAL(activated()), SLOT(musicPlayItemOnce()));

    connect(M_HOTKEY_PTR->getHotKey(10), SIGNAL(activated()), SLOT(musicEnhancedOff()));
    connect(M_HOTKEY_PTR->getHotKey(11), SIGNAL(activated()), SLOT(musicEnhanced3D()));
    connect(M_HOTKEY_PTR->getHotKey(12), SIGNAL(activated()), SLOT(musicEnhancedNICAM()));
    connect(M_HOTKEY_PTR->getHotKey(13), SIGNAL(activated()), SLOT(musicEnhancedSubwoofer()));
    connect(M_HOTKEY_PTR->getHotKey(14), SIGNAL(activated()), SLOT(musicEnhancedVocal()));

    connect(M_HOTKEY_PTR->getHotKey(15), SIGNAL(activated()), qApp, SLOT(quit()));

}

MusicConsoleObject::~MusicConsoleObject()
{
    qDebug() << "\nRelease all!";
    delete m_musicPlayer;
    delete m_musicPlayList;
}

bool MusicConsoleObject::init(const QCoreApplication &app)
{
#ifdef MUSIC_WINEXTRAS
    QCommandLineOption op1("u", "", ".");
    QCommandLineOption op2("d", "", ".");
    QCommandLineOption op3("l", "", ".");

    QCommandLineParser parser;
    parser.addOption(op1);
    parser.addOption(op2);
    parser.addOption(op3);
    parser.process(app);

    if(app.arguments().count() == 1)
    {
        qDebug() << "\nOptions:";
        qDebug() << "-u //Music play url path";
        qDebug() << "-d //Music play dir path";
        qDebug() << "-l //Music playlist url path\n";
        return false;
    }

    if(parser.isSet(op1))
    {
        QString url = parser.value(op1);
        if(url.isEmpty())
        {
            qDebug() << "Music play url path is empty!";
            return false;
        }
        else
        {
            qDebug() << "Add play url path: " << url;
            m_musicPlayList->addMedia(0, url);
            m_musicPlayList->setCurrentIndex(0);
        }
    }
    else if(parser.isSet(op2))
    {
        QString url = parser.value(op2);
        if(url.isEmpty())
        {
            qDebug() << "Music play dir path is empty!";
            return false;
        }
        else
        {
            foreach(const QFileInfo &file, MusicUtils::Core::getFileListByDir(url, MusicFormats::supportFormatsFilterString(), true))
            {
                qDebug() << "Add play url path: " << file.absoluteFilePath();
                m_musicPlayList->appendMedia(0, file.absoluteFilePath());
            }

            if(!m_musicPlayList->isEmpty())
            {
                m_musicPlayList->setCurrentIndex(0);
            }
        }
    }
    else if(parser.isSet(op3))
    {
        QString url = parser.value(op3);
        if(url.isEmpty())
        {
            qDebug() << "Music playlist path is empty!";
            return false;
        }
        else
        {
            if(QFileInfo(url).suffix().toLower() != "lis")
            {
                qDebug() << "Music playlist format not support!";
                return false;
            }

            MusicSongItems items;
            MusicListConfigManager manager;
            if(!manager.readConfig(url))
            {
                qDebug() << "Music playlist read error!";
                return false;
            }
            manager.readMusicSongsConfig(items);

            foreach(const MusicSongItem &item, items)
            {
                foreach(const MusicSong &song, item.m_songs)
                {
                    qDebug() << "Add play url path: " << song.getMusicPath();
                    m_musicPlayList->appendMedia(0, song.getMusicPath());
                }
            }

            if(!m_musicPlayList->isEmpty())
            {
                m_musicPlayList->setCurrentIndex(0);
            }
        }
    }
    else
    {
        qDebug() << "Options error!";
        return false;
    }

    qDebug() << "\nMusic Files count: " << m_musicPlayList->mediaCount() << "\n";

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
#else
    qDebug() << "Qt version less than 5.2 not support commend line!";
#endif

    return app.exec();
}

void MusicConsoleObject::durationChanged(qint64 duration)
{
    print(0, duration);
}

void MusicConsoleObject::positionChanged(qint64 position)
{
    print(position, m_musicPlayer->duration());
}

void MusicConsoleObject::currentIndexChanged(int index)
{
    qDebug() << "\nCurrent Play Indedx: " << index;
    QTimer::singleShot(MT_S2MS, this, SLOT(resetVolume()));
}

void MusicConsoleObject::musicStatePlay()
{
    if(m_musicPlayList->isEmpty())
    {
        return;
    }

    if(m_musicPlayer->isPlaying())
    {
        m_musicPlayer->pause();
    }
    else
    {
        m_musicPlayer->play();
        m_musicPlayer->setVolume(m_volume);
    }
}

void MusicConsoleObject::musicPlayPrevious()
{
    if(m_musicPlayList->isEmpty())
    {
        return;
    }

    if(m_musicPlayList->playbackMode() == MusicObject::PM_PlayRandom)
    {
        m_musicPlayList->setCurrentIndex();
    }
    else
    {
        m_musicPlayer->playPrevious();
    }

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleObject::musicPlayNext()
{
    if(m_musicPlayList->isEmpty())
    {
        return;
    }

    if(m_musicPlayList->playbackMode() == MusicObject::PM_PlayRandom)
    {
        m_musicPlayList->setCurrentIndex();
    }
    else
    {
        m_musicPlayer->playNext();
    }

    m_musicPlayer->play();
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleObject::resetVolume()
{
    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleObject::musicActionVolumeSub()
{
    m_volume = m_musicPlayer->volume();
    m_volume -= 15;
    if(m_volume < 0)
    {
        m_volume = 0;
    }

    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleObject::musicActionVolumePlus()
{
    m_volume = m_musicPlayer->volume();
    m_volume += 15;
    if(m_volume > 100)
    {
        m_volume = 100;
    }

    m_musicPlayer->setVolume(m_volume);
}

void MusicConsoleObject::musicPlayOrder()
{
    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayOrder);
    m_playbackMode = "Order";
}

void MusicConsoleObject::musicPlayRandom()
{
    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayRandom);
    m_playbackMode = "Random";
}

void MusicConsoleObject::musicPlayListLoop()
{
    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayListLoop);
    m_playbackMode = "ListLoop";
}

void MusicConsoleObject::musicPlayOneLoop()
{
    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayOneLoop);
    m_playbackMode = "OneLoop";
}

void MusicConsoleObject::musicPlayItemOnce()
{
    m_musicPlayList->setPlaybackMode(MusicObject::PM_PlayOnce);
    m_playbackMode = "Once";
}

void MusicConsoleObject::musicEnhancedOff()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::EnhancedOff);
    m_enhanced = "Off";
}

void MusicConsoleObject::musicEnhanced3D()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::Music3D);
    m_enhanced = "3D";
}

void MusicConsoleObject::musicEnhancedNICAM()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::MusicNICAM);
    m_enhanced = "NICAM";
}

void MusicConsoleObject::musicEnhancedSubwoofer()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::MusicSubwoofer);
    m_enhanced = "Subwoofer";
}

void MusicConsoleObject::musicEnhancedVocal()
{
    m_musicPlayer->setMusicEnhanced(MusicPlayer::MusicVocal);
    m_enhanced = "Vocal";
}

void MusicConsoleObject::print(qint64 position, qint64 duration)
{
    qDebug() << QString("Music Name: %1, Time:[%2/%3], Volume:%4, PlaybackMode:%5, Enhance:%6")
                .arg(m_musicPlayList->currentMediaString())
                .arg(MusicTime::msecTime2LabelJustified(position))
                .arg(MusicTime::msecTime2LabelJustified(duration))
                .arg(m_musicPlayer->volume())
                .arg(m_playbackMode)
                .arg(m_enhanced);
}
