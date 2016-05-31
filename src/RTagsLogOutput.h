/* This file is part of RTags (http://rtags.net).

   RTags is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   RTags is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RTags.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef RTagsLogOutput_h
#define RTagsLogOutput_h

#include "rct/Connection.h"
#include "rct/Log.h"
#include "rct/String.h"
#include "QueryMessage.h"

class RTagsLogOutput : public LogOutput
{
public:
    RTagsLogOutput(LogLevel level, unsigned int queryFlags, const std::shared_ptr<Connection> &conn = std::shared_ptr<Connection>())
        : LogOutput(level), mFlags(queryFlags), mConnection(conn)
    {
        if (conn) {
            conn->disconnected().connect(std::bind(&RTagsLogOutput::remove, this));
        }
    }

    virtual unsigned int flags() const override { return mFlags; }

    virtual bool testLog(LogLevel level) const override
    {
        if (level == RTags::DiagnosticsLevel && mFlags & QueryMessage::NoSpellChecking)
            return false;
        if (logLevel() < LogLevel::Error || level < LogLevel::Error)
            return level == logLevel();
        return LogOutput::testLog(level);
    }
    virtual void log(Flags<LogFlag>, const char *msg, int len) override
    {
        if (mConnection) {
            std::shared_ptr<EventLoop> main = EventLoop::mainEventLoop();
            if (EventLoop::eventLoop() == main) {
                mConnection->write(String(msg, len));
            } else {
                EventLoop::mainEventLoop()->callLaterMove(std::bind((bool(Connection::*)(Message&&))&Connection::send, mConnection, std::placeholders::_1),
                                                          ResponseMessage(String(msg, len)));
            }
        }
    }
    std::shared_ptr<Connection> connection() const { return mConnection; }
private:
    const unsigned int mFlags;
    std::shared_ptr<Connection> mConnection;
};

#endif
