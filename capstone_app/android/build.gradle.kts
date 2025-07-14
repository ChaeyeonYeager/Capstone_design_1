buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:7.4.2")  // 예시 버전, 프로젝트에 맞게 변경
        classpath("com.google.gms:google-services:4.3.15")
    }
}

// 커스텀 빌드 디렉토리 설정
val newBuildDir = rootProject.layout.buildDirectory.dir("../../build").get().asFile

gradle.beforeProject {
    project.buildDir = File(newBuildDir, project.name)
}

tasks.register<Delete>("clean") {
    delete(rootProject.layout.buildDirectory)
}
