const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const {io: ioClient} = require('socket.io-client');

const PORT = 8001;
const SERVER_NAME = 'Server A';
const PEER_URL = 'http://localhost:8002';

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
    cors: {origin: "*"}
});

const clients = {};

// PEER SERVER
let peer = null;
function connectToPeer() {
    peer = ioClient(PEER_URL, {
        reconnection: true,
        reconnectionDelay: 1000,
    });

    peer.on('connect', () => {
        console.log(`${SERVER_NAME} connected to peer at ${PEER_URL}`);
    });

    peer.on('connect_error', () => {
        //retry
    })
}

connectToPeer();

// LOCAL
io.on('connection', (socket) => {
    console.log(`${SERVER_NAME} client connected: ${socket.id}`);

    socket.on('register', (clientId) => {
        clients[socket.id] = clientId;
        console.log(`${SERVER_NAME} client registered: ${clientId}`, Object.values(clients));
    });

    socket.on('message', (data) => {
        console.log(`${SERVER_NAME} received message from client ${clients[socket.id]}:`, data);

        io.emit('message', data);

        if (peer && peer.connected) {
            peer.emit('peer-message', data);
        }
    });

    socket.on('peer-message', (data) => {
        console.log(`${SERVER_NAME} received message from peer:`, data);
        io.emit('message', data);
    });

    socket.on('disconnect', () => {
        console.log(`${SERVER_NAME} client disconnected: ${socket.id}`);
        delete clients[socket.id];
    });
});

server.listen(PORT, () => {
    console.log(`${SERVER_NAME} listening on port ${PORT}`);
});