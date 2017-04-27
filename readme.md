# Encrypted Chat

This is an assignment for daily course **Internet and World Wide Web** in BNU-HKBU  United International College in semester 2, academic year 2016-2017. 

## Features

The software is based on provided example program, which is a public chatroom base on Winsock library. By doing some modification, the software can apply point-to-point encryption in one-to-one chatting mode. 

* Both encryption and decryption processes are done inside the client side. Routing is done by server. 
* The software uses RSA encryption & decryption algorithm. The implementation refers to http://scanftree.com/programs/c/c-code-to-implement-rsa-algorithmencryption-and-decryption/ .

## Processes

Here are the steps of the program runs.

* Client side
  * Input a username
  * Input two prime numbers
  * Calculate key pair
  * Send hello message to the server with username and keys
  * Refresh the list which contains all online usernames and their keys
  * User can input receiver and message now
    * Client will encrypted message with the public keys of the receiver. 
* Server side
  * Maintain a list of all users with their usernames and keys. 
  * Routing the incoming message.
  * Replying server command.
    * Command **list** will be replied by a list of usernames and keys. 

## Issues

There are quiet a lot issues of this program. 

* Information will be lost during encryption and decryption. This is mainly because the lost of type casting (from `long` to `char`).
* Requires user to input prime numbers by themselves. It should be generated randomly by the software. 
* Types are a mess.

*The repository is now archived and there will not be any further improvement.*