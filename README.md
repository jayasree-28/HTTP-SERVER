# HTTP_Server

# Minimal HTTP Server in C

A basic HTTP/1.1 web server implemented in pure C using the **Winsock API**, capable of handling **GET** and **POST** requests over **TCP/IP**. It serves static HTML pages and supports basic form submissions, making it an ideal learning project to understand how browsers and servers interact at the socket level.

---

## 🔧 Features

- Handles `GET` and `POST` requests from modern browsers  
- Serves static `.html` files from disk  
- Parses and decodes percent-encoded form data (`application/x-www-form-urlencoded`)  
- Recognizes `/quit` in POST body to trigger a clean shutdown  
- Logs client messages and request method to console  
- Modular structure with functions for request parsing, file reading, and client handling

---
▶️ How to Run

Platform: Windows (built using Winsock)

Compiler: Use any C compiler (e.g. gcc, MSVC)

Open your browser and go to: http://localhost:5500

---
🛠 Technologies Used

- C (ISO C89/C99)
- Winsock API
- TCP/IP Sockets
- HTML + basic CSS
- Manual string parsing (no third-party libraries)
