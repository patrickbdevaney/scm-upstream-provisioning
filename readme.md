# Batch Commit Automation

## Purpose
The purpose of the batch commit automation is to streamline the process of provisioning new repositories, cloning them, and uploading large sets of files while adhering to Git's push limits. This is particularly useful for projects with numerous large files that need to be tracked and managed efficiently.

## Applications
This automation can be beneficial in various domains, including but not limited to:
- **Unreal Engine 5 (UE5) Projects**: Managing large assets and ensuring proper version control.
- **Video Production and Editing**: Handling high-resolution video files and project files.
- **3D Modeling and Animation**: Managing large 3D models and animation files.
- **Scientific Research and Data Analysis**: Handling large datasets and ensuring data integrity.
- **Software Development with Large Binary Assets**: Managing large binary files in game development, CAD software, or VR applications.
- **Machine Learning and AI Projects**: Handling large datasets, model files, and training logs.
- **Digital Art and Graphic Design**: Managing high-resolution images and design files.
- **Virtual Reality (VR) and Augmented Reality (AR) Development**: Handling large 3D models and multimedia files.
- **Engineering and CAD Projects**: Managing large design files and ensuring accurate version tracking.

## How to Use

### C File
1. **Compile the C file**:
   - On Linux:
     ```sh
     gcc -o batch_uploader batch_uploader.c
     ```
   - On Windows with w64 devkit:
     ```sh
     gcc -o batch_uploader.exe batch_uploader.c
     ```
   - On Windows with MSVC and Visual Studio:
     - Open the Visual Studio Developer Command Prompt.
     - Navigate to the directory containing the C file.
     - Run:
       ```sh
       cl /Fe:batch_uploader.exe batch_uploader.c
       ```

2. **Place the executable in the root of the repository**.

3. **Ensure `.gitattributes` accounts for LFS** for your directories:
   ```plaintext
   *.ext filter=lfs diff=lfs merge=lfs -text

 4.  **Update .gitignore to make an exception for the executable:**
!batch_uploader

5. **Run the executable:**
./batch_uploader

### Python Script

**Pass the absolute path of the repository in the .py file:**
Python

repo_path = "/absolute/path/to/your/repo"

**Run the Python script from the terminal or Git Bash from the same directory as the .py script:**
python batch_uploader.py

**Ensure .gitattributes accounts for LFS for your directories:**
*.ext filter=lfs diff=lfs merge=lfs -text

## Caveats
While solutions like multithreaded programming can speed up local computation, the overhead of network upload over Git still accounts for most of the runtime and resources. Consequently, a multithreaded implementation won’t have a significant advantage over the linear array implementation in C except in the initial population of the array.

This project is a good starting point to think about tasks that can be parallelized with pthreads, OpenMP, CUDA, and other languages. It helps in determining which use cases best benefit from parallel and concurrent programming.

## Conclusion
By following these steps, you can automate the process of provisioning new repositories, cloning them, and uploading large sets of files efficiently. This automation is useful across various domains and can significantly improve collaboration and version control.