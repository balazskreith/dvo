========================================================================
========================= DEVLEGO IN C++ ===============================
========================================================================

1. What is it?
------------------------------------------------------------------------
devlego in C++ is a toolkit used for realizing flow-based programming in C++.
Flow based programming is a development approach for designing software 
applications by connecting arbitrary processing elements to fulfil the task 
the application is created for.


2. What is it good for?
------------------------------------------------------------------------
The simple explanation that it is good for building applications process
messages in an easy, transparent, maintainable and object-oriented 
manner. It is intended to develop an application by defining black boxes
processing streams and messages between them and with dvo we can easily 
connect these black boxes. 


3. How can I use it?
------------------------------------------------------------------------
Theoretically you need to understand the concept of flow-based 
programming and black boxes. Practically you have to include the dvo.h 
into your project and than inherit and use the classes. 
Some basic example are shown in the examples.cpp source code file.


4. What else should I know?
------------------------------------------------------------------------
Millions of things. To design an application built a network of black 
boxes you need to understand the flow-based designing concept and 
think about decomposition of reusable functionality. 
In dvo.h basic type of black boxes are created. These basic elements are
detailed in section of Basic elements and descriptions of dvo.
A custom notation is developed in order to simplify the design and 
implementation process. You need to aware of push-flow and pull-flow, 
actually you need to know something about data flow model and stream 
processing. There are several things you should know for proper using 
this tool-kit, but right now its enough to see the example. if you 
understand or feel how to use it, than the rest will come.

5. Basic elements and descriptions of dvo
------------------------------------------------------------------------
Elements using push-flow or pull-flow Sockets and Plugs used for 
connections. Those elements use only push-flow are Pushers, and those 
uses only pull-flow are Pullers. If an element uses both type it is 
called Hybrid or Adapters since these type of elements are often used
for connecting a Pusher to a Puller or reverse. In dvo Pushers ar found 
in ps, Pullers are found in pl, Adapters are found in as namespace.

5.1. Pushers
........................................................................
The following basic type of black boxes are found in Pushers:
Transmitter:
Receiver:
Transceiver:
Joiner:
Detacher:
Separator:
Selector:
Restrictor:

5.2. Pullers
........................................................................



5.3. Adapters
........................................................................



6. Building an applicationby using basic type of elements
------------------------------------------------------------------------