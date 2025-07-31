
------------ path ------------
vendor_path = {}
vendor_path["glew"]				= "%{wks.location}/vendor/glew"
vendor_path["glfw"]          	= "%{wks.location}/vendor/glfw"
vendor_path["glm"]           	= "%{wks.location}/vendor/glm"
vendor_path["ImGui"]         	= "%{wks.location}/vendor/imgui"
vendor_path["stb_image"]     	= "%{wks.location}/vendor/stb_image"

------------ include ------------ 
IncludeDir = {}
IncludeDir["glfw"]              = "%{wks.location}/%{vendor_path.glfw}"
IncludeDir["glm"]               = "%{wks.location}/%{vendor_path.glm}"
IncludeDir["ImGui"]             = "%{wks.location}/%{vendor_path.ImGui}"
IncludeDir["tinyobjloader"]     = "%{wks.location}/%{vendor_path.tinyobjloader}"
IncludeDir["stb_image"]         = "%{wks.location}/%{vendor_path.stb_image}"
IncludeDir["ImGuizmo"]          = "%{wks.location}/%{vendor_path.ImGuizmo}"
