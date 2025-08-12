buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:8.4.2")  // 예시 버전, 프로젝트에 맞게 변경
        classpath("com.google.gms:google-services:4.3.15")
        classpath ("org.jetbrains.kotlin:kotlin-gradle-plugin:1.9.24")
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
