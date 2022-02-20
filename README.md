# glvk-interop

A simple demo that shows interoperability between OpenGL and Vulkan.

1. Create an image in Vulkan
2. Share the image memory to OpenGL
3. Render to image on OpenGL
4. Use the shared image for Vulkan rendering
5. Synchronize GL writing and VK reading with shared semaphores