---
layout: post
title:  "Gradle, A Jig"
date:   2015-10-04 00:00:00
categories: craftsmanship
---

In a previous post, I discussed craftsmanship in general terms, focusing on treating the entire stack as production code.  In other words, we should treat it with the same respect as our other efforts that get deployed into production.  Now, I am going to get into specifics on how Gradle can help this effort.  Before starting, I want to say Gradle is not the only tool that can help, but it is the one that I like most.

## The Jig
Craftsmen are lazy and know doing repeated manual tasks leads to mistakes. [Jigs](https://en.wikipedia.org/wiki/Jig_(tool)) allow us to automate manual tasks, making them quicker, more efficient and more uniform. Gradle is a jig, providing the skeleton for better tools.

For this example, our Jig is going to be focused on 'fit and finish'. In Gradle this means that all the plugins act like you expect, with sane defaults, and the same configuration across the project.

Gradle provides this through [`withType`](https://docs.gradle.org/current/javadoc/org/gradle/api/DomainObjectCollection.html#withType(java.lang.Class,%20org.gradle.api.Action)) and [`matching`](https://docs.gradle.org/current/javadoc/org/gradle/api/DomainObjectCollection.html#matching(org.gradle.api.specs.Spec)) as part of the base collection container. With them we can easily auto configure domain objects. For example, if your project uses checkstyle it should be auto configured and just work for the user.

Lets assume you have a plugin called [`corp-base`](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-corpbaseplugin-groovy) and all plugins you write will apply it.

### Code Quality
Gradle requires a configuration file for the checkstyle plugin, without it the build will fail. For this example lets use [Google's Style Guide](https://raw.githubusercontent.com/checkstyle/checkstyle/master/src/main/resources/google_checks.xml). In an effort to delight our users will configure checkstyle out of the box, creating a custom task that writes the checkstyle config file to disk.

    @TaskAction
    public void copyConfig() {
        if(getConfigDestination().exists()) {
            return;
        }
    
        getConfigDestination().text = this.getClass().getClassLoader().getResource("com/example/checkstyle-default.xml").text;
    }

The custom task is called [CheckstyleCopyConfigTask](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-checkstylecopyconfigtask-groovy). Lets break down the code sample from above. The `@TaskAction` tells Gradle that the `copyConfig` method should be called when the task is executed. The first thing we do is to check for the existence of the config file. We do this to make sure that if our user want's to define their own style guide they can. If the file doesn't exist we will save our default.

    private void configureCheckstyle(Project project) {
        project.plugins.apply(CheckstylePlugin)
    
        CheckstyleCopyConfigTask configureCheckstyle = createOrGetCheckstyleConfigTask(project)
        project.getExtensions().findByType(CheckstyleExtension).toolVersion = '6.7'
        project.tasks.withType(Checkstyle, new CheckstyleTaskAction(configureCheckstyle))
    }
     
    private CheckstyleCopyConfigTask createOrGetCheckstyleConfigTask(Project project) {
      return project.rootProject.getTasks().maybeCreate('configureCheckstyle, CheckstyleCopyConfigTask)
    }

The [CorpBasePlugin](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-corpbaseplugin-groovy) does a few things. First the plugin applies the [`CheckstylePlugin`](https://docs.gradle.org/current/userguide/checkstyle_plugin.html) so our users don't have to. The plugin also creates a [CheckstyleCopyConfigTask](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-checkstylecopyconfigtask-groovy) task called `configureCheckstyle`. Next the plugin sets the checkstyle version to `6.7`, providing a more recent default. Finally the plugin uses `withType` to apply [CheckstyleTaskAction](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-checkstyletaskaction-java) to all `Checkstyle` tasks. The CheckstyleTaskAction makes all of the `Checkstyle` tasks depend on `configureCheckstyle` and set the configFile to be `CheckstyleCopyConfigTask.getConfigDestination()`.

### Testing
Now we have some code that might work, instead of doing TDD I use what my co-worker [Baron Roberts](https://www.linkedin.com/pub/baron-roberts/0/159/65b) calls 'lock-step-development'. Using lock-step-development you write a test at the point that you wonder 'does this work?'. For testing I use the [Spock Framework](spockframework.github.io/spock/docs) an extension of Junit. When I need to test Gradle code, I also like to use Netflix's [nebula-test](https://github.com/nebula-plugins/nebula-test); an extension of Spock that is focused on Gradle testing. As of Gradle 2.6, Gradle Inc. has started to provide a testing framework, but it doesn't work if you need to use older versions of Gradle. For now, I'm sticking with nebula-test.

    class CorpBasePluginTest extends PluginProjectSpec

The first test is a simple one using nebula. By extending [PluginProjectSpec](https://github.com/nebula-plugins/nebula-test#pluginprojectspec) you get some sanity checks. 

> adds three tests that ensure the plugin can be applied properly (idempotently and in a multi-project).

Lets also add another test to make sure that were using the right version of checkstyle.

  
    def 'checkstyle tool is configured'() {
        when:
        project.apply plugin: pluginName
    
        then:
        project.getExtensions().findByType(CheckstyleExtension).toolVersion == '6.7'
    }

Unit tests are great (take a look at it [here](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-corpbaseplugintest-groovy)). They are quick and let you iterate quickly. However, sometimes you need to interact with Gradle's lifecycle to make sure that everything works as you expect. Luckily our friends at Netflix have also provided a class called [IntegrationSpec](https://github.com/nebula-plugins/nebula-test#integrationspec). This guy is your multi-tool; he can do just about anything you need, but takes a little time to do it. 

    def setup() {
      buildFile << createDefaultBuildFileContents()
      writeHelloWorld('com.example')
    }
    
    def 'can run checkstyle on root project'() {
      when:
      def result = runTasksSuccessfully('checkstyleMain')
    
      then:
      result.wasExecuted('checkstyleMain')
      result.wasExecuted(CorpBasePlugin.ROOT_CHECKSTYLE_TASK)
    }

This integration tests (found at [CorpBasePluginIntegrationTest](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-corpbasepluginintegrationtest-groovy)) sets up a simple project, and runs `checkstyleMain`. Again this runs in a gradle daemon and writes to the filesystem, so it's slow.

## Binary Distribution
Now our jig is able to configure checkstyle, and we have tests that assert that it works as expected. Now we want to let others use it! In a normal project we would define a dependency on it and let the Gradle manage the dependencies. Luckily, Gradle lets us do the _exact same thing_ for our plugins by using the [buildscript](https://docs.gradle.org/current/userguide/organizing_build_logic.html#sec:external_dependencies) section.

**Note:** If you want to follow along you can clone this [gist](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69). From inside the directory run `./setup.sh`. Once complete you should have a working project. This project retrieves a [Vagrantfile](https://gist.github.com/ethankhall/dbd5abeb7971ee39b408#file-vagrantfile) that I use to run a free Artifactory instance.

Taking a look at the [build.gradle](https://gist.github.com/ethankhall/66aa4a50c9bb79b40d69#file-build-gradle), we can see the publishing section:

    publishing {
        publications {
            maven(MavenPublication) {
                groupId = 'com.example'
                artifactId = 'awesome-plugins'
                from components.java
            }
        }
        repositories {
            maven {
                url "http://10.1.1.123:8081/artifactory/ext-release-local"
                credentials {
                    username "admin"
                    password "password"
                }
            }
        }
    }

If you bring up the Artifactory instance with `vagrant up` (it may take a few min) you can run `./gradlew publish` you should see something like:

    :gradle-plugin:generatePomFileForMavenPublication
    :gradle-plugin:compileJava UP-TO-DATE
    :gradle-plugin:compileGroovy
    :gradle-plugin:processResources
    :gradle-plugin:classes
    :gradle-plugin:jar
    :gradle-plugin:publishMavenPublicationToMavenRepository
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.jar
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.jar.sha1
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.jar.md5
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.pom
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.pom.sha1
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/1.0.0/awesome-plugins-1.0.0.pom.md5
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/maven-metadata.xml
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/maven-metadata.xml.sha1
    Upload http://10.1.1.123:8081/artifactory/ext-release-local/com/example/awesome-plugins/maven-metadata.xml.md5

You can check that the artifact is in Artifactory using this [link](http://10.1.1.123:8081/artifactory/simple/ext-release-local/com/example/awesome-plugins/1.0.0/).

Now that the plugins are published, lets use it. Uncomment the section in the build.gradle file, you will see that we declare a `buildscript`. 

    apply plugin: 'corp-base'
    buildscript {
        repositories {
            maven {
                url "http://10.1.1.123:8081/artifactory/ext-release-local"
            }
        }
        dependencies {
            classpath 'com.example:awesome-plugins:1.0.0'
        }
    }

The buildscript declares a repository on our Artifactory instance and adds our plugin jar to Gradle. Now we can apply our plugin `corp-base`, this will configure the project per the plugin. If you were to run `./gradlew checkstyleMain ` you should see that `configureCheckstyle` also ran and that a file `config/checkstyle/corp-checkstyle.xml` exists.

## Jigging a Jig
For this walk through, we created a base plugin that others can apply. Following the [Single responsibility principle](https://en.wikipedia.org/wiki/Single_responsibility_principle), we have a task that writes the configuration file if needed. We then configure all Checkstyle tasks to have a dependency on the custom task and use the custom file. We wrote a few tests using Spock and nebula-test to ensure that our code works. We then upload the artifacts to Artifactory, allowing us to dogfood our own plugin.

We leveraged Gradle's power to help a whole organization raise their craftsmanship. By dogfooding/bootstrapping plugins you can ensure that the users can use the project and work as you expect. Testing the plugins keeps the quality high. Following coding styles and code reviews keeps the bus factor high. When these things all come together craftsmanship starts to take hold.

