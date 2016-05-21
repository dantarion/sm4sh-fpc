import SimpleHTTPServer
import SocketServer
import zlib,os,struct
class FPCServer(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def generate_cachesize(self):
            size = 0
            for root,dirs,files in os.walk("."):
                for fname in files:
                    size += 4
            self.send_response(200)
            self.send_header('Content-Length', size)
            self.end_headers()
            pass
    def generate_cache(self):
        size = 0
        for root,dirs,files in os.walk("."):
            for fname in files:
                s = zlib.crc32(os.path.join(root,fname)[2:]+"\n")
                print os.path.join(root,fname)[2:]
                s = struct.pack("I",zlib.crc32(os.path.join(root,fname)[2:]) & 0xFFFFFFFF)

                self.wfile.write(s)
        pass
    def do_HEAD(self):
        if self.path == "/cache":
            return self.generate_cachesize()
        return SimpleHTTPServer.SimpleHTTPRequestHandler.do_HEAD(self)
    def do_GET(self):
        if self.path == "/cache":
            return self.generate_cache()
        return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)
PORT = 7000

Handler = FPCServer

httpd = SocketServer.TCPServer(("", PORT), Handler)

print "serving at port", PORT
httpd.serve_forever()
