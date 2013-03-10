foo\_http\_now_playing.dll
====
 
A foobar2000 plugin, it will make an **HTTP POST** to a specified URL of what currently is playing. Based on SDK-2011-03-11. 
  
URL must be set in **"configuration -> display -> HTTP Now Playing"**.  
 
Download
====
[https://github.com/Adios/foo\_http\_now_playing/downloads](https://github.com/Adios/foo_http_now_playing/downloads)
 
depends on _VC++ 2010 x86 redistributable_, which will be there if you had installed https://github.com/rdp/virtual-audio-capture-grabber-device before. 

What server side got
====
HTTP server will receive the POST message with the following 5 fields: (explaining in PHP)

    $_POST['album'];
    $_POST['artist'];
    $_POST['title'];
    $_POST['filename_ext'];
    $_POST['playlist'];
