pipeline {
    agent {
		dockerfile {
			filename 'Dockerfile'
			dir 'docker'
			additionalBuildArgs '--rm -t jenkins:totem' + env.BRANCH_NAME.replace('/', '_').toLowerCase()
		}
	}

    stages {
        stage('git') {
            steps {
                dir('source'){
                    echo 'getting the code...'
                    checkout scm
                    script {
                        GIT_VERSION = sh(script: 'git describe --always --dirty --tags', returnStdout:true).trim()
                        GIT_DESCR = sh(script: 'git log --pretty=format:"%h %an: %s" -1', returnStdout:true).trim()
                        currentBuild.displayName = "#${BUILD_NUMBER}  (${GIT_VERSION})"
                        currentBuild.description = "last commit: ${GIT_DESCR}"
                    }
                }
            }
        }
        stage('build Totem') {
            steps {
                dir('source/application') {
                    echo 'cleaning...'
                    sh 'rm -rf $(find . -name "CMakeFiles" -o -name "CMakeCache.txt" -o -name "Makefile" -o -name "cmake_install.cmake" -o -name "Release")'
                    echo 'building...'
                    sh 'cmake .'
                    sh 'make'
                }
            }
        }
        stage('archive') {
            steps {
                echo 'archiving...'
                archiveArtifacts artifacts: 'source/application/Release/*.bin', onlyIfSuccessful: true
            }
        }
    }
}
