// glasscoder.h
//
// glasscoder(1) Audio Encoder
//
//   (C) Copyright 2014-2015 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef GLASSCODER_H
#define GLASSCODER_H

#include <stdint.h>

#include <vector>

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "audiodevice.h"
#include "codec.h"
#include "config.h"
#include "connector.h"
#include "fileconveyor.h"
#include "glasslimits.h"
#include "metaserver.h"
#include "ringbuffer.h"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void audioDeviceStoppedData();
  void connectorStoppedData();
  void meterData();
  void connectedData(bool state);
  void exitTimerData();

 private:
  Config *sir_config;
  //
  // Audio Device
  //
  bool StartAudioDevice();
  std::vector<Ringbuffer *> sir_ringbuffers;
  AudioDevice *sir_audio_device;

  //
  // Server Connection
  //
  void StartServerConnection(const QString &mntpt="",bool is_top=false);
  std::vector<Connector *> sir_connectors;
  FileConveyor *sir_conveyor;

  //
  // Codec
  //
  bool StartCodec();
  std::vector<Codec *> sir_codecs;

  //
  // Metadata Processor
  //
  MetaServer *sir_meta_server;

  //
  // Miscelaneous
  //
  bool StartSingleStream();
  bool StartMultiStream();
  QTimer *sir_meter_timer;
  QTimer *sir_exit_timer;
  unsigned sir_exit_count;
};


#endif  // GLASSCODER_H
