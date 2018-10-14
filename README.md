
### Http-server
This project creates a static and a dynamic web server using the socket package in C. 

- It will only support the GET method.  If a browser sends other
  methods (POST, HEAD, PUT, for example), the server responds with the
  status code 501.  Here is a possible response: HTTP/1.0 501 Not Implemented
  <html><body><h1>501 Not Implemented</h1></body></html>
  Note that the server adds a little HTML body for the status code and
  the message.  Without this, the browser will display a blank page.
  This should be done for all status codes except 200.
  
- Our server will be strictly HTTP 1.0 server.  That is, all responses
  will say "HTTP/1.0" and the server will close the socket connection
  with the client browser after each response.
  The server will accept GET requests that are either HTTP/1.0 or
  HTTP/1.1 (most browsers these days sends HTTP/1.1 requests).  But it
  will always respond with HTTP/1.0.  The server should reject any
  other protocol and/or version, responding with 501 status code.
  
- The server should also check that the request URI (the part that
  comes after GET) starts with "/".  If not, it should respond with
  "400 Bad Request".
  
- In addition, the server should make sure that the request URI does not
  contain "/../" and it does not end with "/.." because allowing ".." in the
  request URI is a big security risk--the client will be able to fetch a
  file outside the web root.
