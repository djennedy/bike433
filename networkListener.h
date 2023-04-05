#ifndef NETWORKLISTENER_H
#define NETWORKLISTENER_H

/**
 * Module to listen to commands sent via UDP in its own thread
*/

// Starts listening on fixed port 12345
void Net_startListening();

// Stops listening and cleans up all memory used
void Net_stopListening();


#endif /* NETWORKLISTENER_H */
