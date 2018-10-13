This file should contain:

  - your name
  - your UNI
  - lab assignment number
  - description for each part
  
The description should indicate whether your solution for the part is
working or not.  You may also want to include anything else you would
like to communicate to the grader such as extra functionalities you
implemented or how you tried to fix your non-working code.

------

Name: Wu Po Yu
UNI: pw2440
lab 7 

All parts work as intended, with four kinds of responses to the browser, 404
Not Found, 400 Bad Request, 500 Not Implemented, and 200 OK.

If the program is terminated with an ctrl-C, the backend socket will not be
freed, which would result in reachable error in valgrind 
