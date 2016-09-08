/*=========================================================================

  Program:   Birch
  Module:    QBirchVTKOutputWindow.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

  * adapted from ...

  Copyright (c) 2016 David Gobbi
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

  * Neither the name of the Calgary Image Processing and Analysis Centre
    (CIPAC), the University of Calgary, nor the names of any authors nor
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/
#include <QBirchVTKOutputWindow.h>

// Qt includes
#include <QtGlobal>

QBirchVTKOutputWindow *QBirchVTKOutputWindow::New()
{
  return new QBirchVTKOutputWindow;
}

QBirchVTKOutputWindow::QBirchVTKOutputWindow()
{
}

QBirchVTKOutputWindow::~QBirchVTKOutputWindow()
{
}

void QBirchVTKOutputWindow::Initialize()
{
}

void QBirchVTKOutputWindow::Install()
{
  QBirchVTKOutputWindow *win = QBirchVTKOutputWindow::New();
  vtkOutputWindow::SetInstance(win);
  win->Delete();
}

void QBirchVTKOutputWindow::DisplayText(const char* text)
{
  if (text)
    {
#if QT_VERSION >= 0x050000    
    qInfo("%s", text);
#else
    qWarning("%s", text);
#endif   
    }
}

void QBirchVTKOutputWindow::DisplayErrorText(const char* text)
{
  if (text)
    {
    qCritical("%s", text);
    }
}

void QBirchVTKOutputWindow::DisplayWarningText(const char* text)
{
  if (text)
    {
    qWarning("%s", text);
    }
}

void QBirchVTKOutputWindow::DisplayGenericWarningText(const char* text)
{
  if (text)
    {
    qWarning("%s", text);
    }
}

void QBirchVTKOutputWindow::DisplayDebugText(const char* text)
{
  if (text)
    {
    qDebug("%s", text);
    }
}

void QBirchVTKOutputWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
