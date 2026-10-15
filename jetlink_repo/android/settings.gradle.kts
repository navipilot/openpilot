pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        // LiteRT's NPU runtime libraries, which LiteRT publishes as a release
        // asset rather than on Maven; nothing else is looked for there.
        exclusiveContent {
            forRepository {
                ivy {
                    name = "LiteRT releases"
                    url = uri("https://github.com/google-ai-edge/LiteRT/releases/download")
                    patternLayout { artifact("v[revision]/[artifact].[ext]") }
                    metadataSources { artifact() }
                }
            }
            filter { includeModule("com.google.ai.edge.litert", "litert_npu_runtime_libraries_jit") }
        }
    }
}

rootProject.name = "Jetlink"
include(":app")
