# Medical Scheduler System

## Purpose
The Medical Scheduler System is a C++ client-server application designed to manage medical appointments in a hospital or clinic setting. It allows patients to securely sign up, sign in, and request appointments, while providing administrators with tools to approve or reject these appointments. The system supports multiple clients, secure communication, and a viewer for all appointment statuses.

## How It Works
- **Server (`server.exe`)**: Listens for client connections, manages user accounts and appointments, and provides an interactive admin menu for appointment management (list, approve, reject) directly from the server console.
- **Client (`client.exe`)**: Used by patients to sign up, sign in, and request appointments. Communicates with the server over TCP sockets.
- **Viewer (`approved_viewer.exe`)**: Connects to the server and displays all appointments (approved, rejected, pending) in a user-friendly format.

### Key Features
- **Secure Sign-Up/Sign-In**: Passwords are encrypted using the Vigenère cipher before being sent to the server.
- **Appointment Requests**: Patients can request appointments with details such as doctor, date/time, and reason.
- **Admin Approval/Rejection**: Admins can approve or reject appointments either via network commands or directly from the server's interactive console menu.
- **Multi-Client Support**: The server can handle multiple clients at the same time.
- **Viewer**: See all appointments and their statuses in one place.
- **Portable Executables**: All `.exe` files are statically linked and can be run on any Windows PC without extra DLLs.

## File Tree
```
medical-scheduler/
├── CMakeLists.txt
├── README.md
├── build/
│   └── Debug/
│       ├── client.exe
│       ├── server.exe
│       └── approved_viewer.exe
├── src/
│   ├── client/
│   │   ├── main.cpp
│   │   ├── Patient.cpp
│   │   ├── Patient.h
│   │   ├── VigenereCipher.cpp
│   │   ├── VigenereCipher.h
│   │   ├── NetworkClient.cpp
│   │   └── NetworkClient.h
│   ├── server/
│   │   ├── main.cpp
│   │   ├── Server.cpp
│   │   ├── Server.h
│   │   ├── Admin.cpp
│   │   ├── Admin.h
│   │   ├── Appointment.cpp
│   │   └── Appointment.h
│   ├── viewer/
│   │   ├── main.cpp
│   │   ├── ApprovedAppointmentsViewer.cpp
│   │   └── ApprovedAppointmentsViewer.h
│   ├── common/
│   │   ├── User.h
│   │   ├── User.cpp
│   │   ├── Protocol.h
│   │   └── Protocol.cpp
│   └── utils/
│       ├── EncryptionUtils.h
│       └── EncryptionUtils.cpp
```

## How to Build
1. **Install CMake and a C++ compiler** (Visual Studio recommended on Windows).
2. **Open a terminal in the project root.**
3. **Run:**
   ```powershell
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Debug
   ```
4. **Find the executables in `build/Debug/`:**
   - `client.exe`
   - `server.exe`
   - `approved_viewer.exe`

## How to Run
1. **Start the server:**
   ```powershell
   ./server.exe
   ```
   - The server will show its IP and port.
   - The admin menu will appear in the server console for listing, approving, and rejecting appointments.
2. **Run one or more clients:**
   ```powershell
   ./client.exe
   ```
   - Use to sign up, sign in, and request appointments.
3. **Run the viewer:**
   ```powershell
   ./approved_viewer.exe
   ```
   - Shows all appointments and their statuses (approved, rejected, pending).

## Example Workflow
1. Start `server.exe`.
2. Use `client.exe` to sign up and request appointments.
3. Use the server's admin menu to approve/reject appointments.
4. Run `approved_viewer.exe` to see all appointments and their statuses.

## Project Details
- **Patient Workflow:**
  - Sign up or sign in using the client.
  - Request an appointment by providing doctor, date/time, and details.
- **Admin Workflow:**
  - Use the server's admin menu to list all appointments, approve, or reject them by ID.
  - Alternatively, admin actions can be sent as network commands from a client.
- **Viewer:**
  - Connects to the server and displays all appointments with their status.
- **Security:**
  - Passwords are encrypted with the Vigenère cipher before being sent to the server.
- **Portability:**
  - All executables are statically linked and can be run on any Windows PC without extra DLLs.

## Troubleshooting
- **If client.exe fails to connect:**
  - Make sure the server is running and reachable on the network.
  - Check firewall settings.
- **If you see missing DLL errors:**
  - All executables are statically linked, but if you see errors, rebuild as described above.
- **For any issues:**
  - Check the logs in the server console.
  - Ensure all executables are from the latest build.

---
For questions or issues, please contact the project maintainer or open an issue.