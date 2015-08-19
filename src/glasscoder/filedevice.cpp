// filedevice.cpp
//
// Audio source for streaming direct from a file.
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

#include "filedevice.h"
#include "glasslimits.h"

FileDevice::FileDevice(unsigned chans,unsigned samprate,
		       std::vector<Ringbuffer *> *rings,QObject *parent)
  : AudioDevice(chans,samprate,rings,parent)
{
#ifdef SNDFILE
  file_sndfile=NULL;
  memset(&file_sfinfo,0,sizeof(file_sfinfo));

  file_read_timer=new QTimer(this);
  connect(file_read_timer,SIGNAL(timeout()),this,SLOT(readTimerData()));
#endif  // SNDFILE
}


FileDevice::~FileDevice()
{
#ifdef SNDFILE
  delete file_read_timer;
  if(file_sndfile!=NULL) {
    sf_close(file_sndfile);
  }
#endif  // SNDFILE
}


bool FileDevice::processOptions(QString *err,const QStringList &keys,
				const QStringList &values)
{
#ifdef SNDFILE
  for(int i=0;i<keys.size();i++) {
    bool processed=false;
    if(keys[i]=="--file-name") {
      file_name=values[i];
      processed=true;
    }
    if(!processed) {
      *err=tr("unrecognized option")+" "+keys[i]+"\"";
      return false;
    }
  }
  if(file_name.isEmpty()) {
    char filename[256];

    scanf("%255s",filename);
    file_name=filename;
  }
  return true;
#else
  *err=tr("device not supported");
  return false;
#endif
}


bool FileDevice::start(QString *err)
{
#ifdef SNDFILE
  if((file_sndfile=sf_open(file_name.toUtf8(),SFM_READ,&file_sfinfo))==NULL) {
    *err=sf_strerror(file_sndfile);
    return false;
  }

  if(file_sfinfo.channels>MAX_AUDIO_CHANNELS) {
    *err=tr("unsupported channel count");
    sf_close(file_sndfile);
    return false;
  }

  file_read_timer->start(1000*SNDFILE_BUFFER_SIZE/file_sfinfo.samplerate);
  return true;
#else
  return false;
#endif  // SNDFILE
}


unsigned FileDevice::deviceSamplerate() const
{
#ifdef SNDFILE
  return file_sfinfo.samplerate;
#else
  return DEFAULT_AUDIO_SAMPLERATE;
#endif  // SNDFILE
}


void FileDevice::readTimerData()
{
#ifdef SNDFILE
  float pcm1[SNDFILE_BUFFER_SIZE*MAX_AUDIO_CHANNELS];
  float pcm2[SNDFILE_BUFFER_SIZE*MAX_AUDIO_CHANNELS];
  float *pcm=pcm1;
  sf_count_t nframes;

  if((nframes=sf_readf_float(file_sndfile,pcm1,SNDFILE_BUFFER_SIZE))>0) {
    if(file_sfinfo.channels!=(int)channels()) {
      remixChannels(pcm2,channels(),pcm1,file_sfinfo.channels,nframes);
      pcm=pcm2;
    }
    for(unsigned i=0;i<ringBufferQuantity();i++) {
      ringBuffer(i)->write(pcm,nframes);
    }
  }
  else {
    sf_close(file_sndfile);
    file_sndfile=NULL;
    emit hasStopped();
  }
#endif  // SNDFILE
}