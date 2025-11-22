# EdgeDetectr

EdgeDetectr is a full-stack web application for applying image processing operators (Sobel, Prewitt, Roberts, etc.) to uploaded images. The project consists of:

- **Frontend**: A Next.js application for user interaction.
- **Backend**: An Express.js server that handles image uploads and processing.
- **Operators**: A C++ module using OpenCV to apply edge detection operators.
- **Deployment**: 
  - Backend: Containerized using Docker and deployed to AWS ECS
  - Frontend: Deployed to AWS Amplify

## 🎥 Demo
https://github.com/user-attachments/assets/66095fc9-8ccd-42ec-b354-fcd836d3d92c

## 🚀 Features
- Upload an image via the frontend.
- Choose an edge detection operator (Sobel, Prewitt, Roberts, etc.).
- Process the image on the backend using C++ and OpenCV.
- Retrieve and display the processed image.
- Rate limiting to prevent abuse (maximum 1 upload every 30 seconds).
- Fully containerized for easy deployment.

---

## 🛠️ Installation

### Prerequisites
- **Docker** (for containerization)
- **Docker Compose** (to manage multi-container applications)

### Clone the Repository
```sh
# Clone main repo and submodules
git clone --recurse-submodules https://github.com/your-org/EdgeDetectr-Meta.git
cd EdgeDetectr-Meta
```

### Build & Run Containers Locally
```sh
docker-compose up --build
```
This runs **frontend, backend, and operators** in separate containers.



## 📌 API Routes

### **Upload an Image & Process with Operator**
**`POST /operators/:operator`**
```sh
curl -X POST -F "file=@image.jpg" http://localhost:3001/operators/sobel
```
Response:
```json
{
  "status": "success",
  "output": "/uploads/output-image.jpg"
}
```

> Note: The API is protected by a client-side rate limit that allows only one image upload per 30 seconds.

---

## 🛠️ Tech Stack
- **Frontend**: Next.js, React, Tailwind CSS
- **Backend**: Node.js, Express.js, Multer (file uploads)
- **Image Processing**: OpenCV (C++), CMake
- **Database**: Local storage (future implementation for S3 support)
- **Security**: Client-side rate limiting for DDoS protection
- **Containerization**: Docker, Docker Compose

---

## 📜 License
MIT License.

---

## ✨ Contributors
- **Kailin Xing** - Developer
- **Kailin Xing** - Maintainer

PRs & Issues are welcome! 🚀
