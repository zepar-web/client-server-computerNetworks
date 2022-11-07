# client-server-computerNetworks

A program that is divided into client and server. They communicate with each other through a FIFO and inside the server is also used for communication father - son pipe and socketpair.

The following commands are implemented in the program:
login: |
logout |
get-logged-users |
get-proc-info: -> show details about a particular process (ps -aux) |
quit |

The idea behind it is the following.
The command is written in the client, the server perceives the command and on the father process is taken over, the son processes it and sends the output to the father to be sent to the client back and displayed to the user.
