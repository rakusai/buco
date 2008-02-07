#
#  AppController.rb
#  buco_osx
#
#  Created by ujihisa on 07/12/21.
#  Copyright (c) 2007 __MyCompanyName__. All rights reserved.
#

require 'osx/cocoa'
require 'net/http'

class AppController < OSX::NSObject
  include OSX
  ib_outlet :window

  def awakeFromNib
    @window.alphaValue = 0.8
  end

  def onStart
    filename = '/tmp/buco.png'

    Thread.start do
      loop do
        system "screencapture -xC -t png -T 0 #{filename}"
        buco = NSImage.alloc.
          initWithContentsOfFile filename
        x, y = NSEvent.mouseLocation.x - 58, NSEvent.mouseLocation.y - 58
        buco.lockFocus
        buco2 = NSBitmapImageRep.alloc.
          initWithFocusedViewRect [x, y, 116, 116]
        buco.unlockFocus

        properties = NSDictionary.dictionaryWithObject_forKey(NSNumber.numberWithBool(true), NSImageInterlaced)
        NSBitmapImageRep.imageRepWithData(buco2.TIFFRepresentation).
          representationUsingType_properties(NSPNGFileType, properties).
          writeToFile_atomically(filename,true)
        send_buco filename
        p 'ok'
        sleep 5.0
      end
    end
  end
  ib_action :onStart

  def send_buco(filename)
    boundary = 'ujihisaujihisaujihisa'
    contents = File.open(filename, 'rb') {|f| f.read }
    data     = <<-END.gsub(/^\s+\|/, '')
      |--#{boundary}
      |Content-Disposition: form-data; name="name"
      |
      |deguchi
      |--#{boundary}
      |Content-Disposition: form-data; name="speed"
      |
      |0.0
      |--#{boundary}
      |Content-Disposition: form-data; name="data"; filename="#{filename}"
      |Content-Type: image/png
      |
      |#{contents}
      |
      |--#{boundary}--
    END
    header = {
      'Content-Length' => data.length.to_s,
      'Content-Type' => "multipart/form-data; boundary=#{boundary}"
    }

    Thread.start do
      h = Net::HTTP.new 'sharestyle.com', 80
      h.post '/~rakusai/buco/buco.php', data, header
    end
  end
  private :send_buco
end

