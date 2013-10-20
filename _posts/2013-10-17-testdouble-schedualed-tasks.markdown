---
layout: post
title:  "Test Double for @Schedualed tasks"
date:   2013-10-17 18:49:32
categories: spring anotations gradle jetty
---
I ran into an interesting issue that I think others might benifit from knowing about. I am writing some code that needs to run at a time to do a schedualed job. Much like a cron job would do. I am having a hard time testing it inside the software package that it is in now, do I am going to make a "[Test Double](http://en.wikipedia.org/wiki/Test_double)", not really a by the definition, but close.

# TL;DR 
Clone the full example from [Github](https://github.com/ethankhall/mule).

# Requirements
Do do this I want to be able to use code that is being created in another jar. So this project will only contain the code needed to run the code that's being worked on. So I am going to use [Gradle](http://www.gradle.org/) to help out. (As you will see, I like gradle). I am also going to use [Jetty](http://www.eclipse.org/jetty/) as a web container. This will let me serve up the content. It will also allow us to make RESTful calls into the instance for testing. I will be using [Spring](http://spring.io/), for doing the bean creating and management.

# Gradle
## Basic Setup
We are going to add the [Gradle Wrapper](http://www.gradle.org/docs/current/userguide/gradle_wrapper.html) to the project, so that others can use it. I usually like to put this at the very end of the file.

{% highlight groovy %}
task wrapper(type: Wrapper) {
    gradleVersion = '1.8'
}
{% endhighlight %}

We are going to need to add the following to tell Gradle how to build the application and run Jetty. To do this we are going to need to add two lines. I usually put it at the top of the file.

{% highlight groovy %}
apply plugin: 'war'
apply plugin: 'jetty'
{% endhighlight %}

Finally to set up the basics to build. Adding Spring to the dependencies. Check the gist to see what the dependencies are.

## Full file used
{% gist ethankhall/4a6ae52d5348f56f6857 build.gradle %}

#Java
##Spring Config
The Spring config is pretty simple. All that is required is the [@Configuration](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/context/annotation/Configuration.html). This will allow us to point the web.xml at a java class and start the beanification. If you are starting on a newer project and are using Spring 3.0 or better you should use the [Componenet Scan](http://docs.spring.io/spring/docs/3.0.0.M3/reference/html/ch04s12.html) funcionality to to create the beans vs using xml.

To have spring scan your classes and create the beans automatically you need to do two things.

1.  Add a [Componenet Stereotype](http://docs.spring.io/spring/docs/3.2.x/javadoc-api/org/springframework/stereotype/Component.html) annotation to the class that you want a bean of. There are four that can be used, they are:

  1.  [@Component](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/stereotype/Component.html)
  2.  [@Service](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/stereotype/Service.html)
  3.  [@Repository](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/stereotype/Repository.html)
  4.  [@Controller](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/stereotype/Controller.html)

2.  Add the [@ComponentScan](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/context/annotation/ComponentScan.html) annotation to the top of your config class. This has a parameter, it is the base class that you want scanned. In this example it is *io.ehdev.mule*. So the code for this would look like
{% highlight java %}
@ComponentScan({ "io.ehdev.mule" })
{% endhighlight %}

For us to be able to use the JSON generateion, we need to enable spring WebMcv. This is pretty easy by using the annotation [@EnableWebMvc](http://docs.spring.io/spring/docs/3.1.4.RELEASE/javadoc-api/org/springframework/web/servlet/config/annotation/EnableWebMvc.html). This will allow us to return JSON from a RESTful request. This also allows us to use RESTful requests to the application. For more info beyond the basic info you can look at [String Documentation](http://docs.spring.io/spring/docs/3.2.x/spring-framework-reference/html/mvc.html)

Finally, to do what we this project is for we need to add two more annotations (we have a lot of annotations, but it's better than XML). The two annotations we need to add enable schedualing and async tasks. The annotations are [@EnableAsync](http://docs.spring.io/spring/docs/3.1.4.RELEASE/javadoc-api/org/springframework/scheduling/annotation/EnableAsync.html) and [@EnableScheduling](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/scheduling/annotation/EnableScheduling.html).

Finally we have out class that can be used to start up the server. I put this file in src/main/java/io.ehdev.mule.config.

{% gist ethankhall/4a6ae52d5348f56f6857 InitalConfig.java %}

## Working
Now that our conext is set up, we can set up the actual componenets needed to make the mule work. There are two componenets we are going to make, the first does the work, the second is so we can return some JSON with the REST call.

Take a look at the file that does the work.
{% gist ethankhall/4a6ae52d5348f56f6857 SampleSchedule.java %}
You probably noticed a few annotations there. Let's go through what they do and how they are awesome.

1.  @Controller - Will create the bean for us since we are using ComponentScanning
2.  [@RequestMapping](http://docs.spring.io/spring/docs/3.1.3.RELEASE/spring-framework-reference/htmlsingle/spring-framework-reference.html#mvc-ann-requestmapping) - Tells us how to map the REST requests to method calls. You can have one on a class, and one on a method and they will append. So in this example you would call ROOT/sample/runManually to get to that method call.
3.  [@Scheduled](http://docs.spring.io/spring/docs/3.1.0.RC1/spring-framework-reference/html/scheduling.html) - Allows a method to be called at a specific intergral.
4.  [@ResponseBody](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/web/bind/annotation/ResponseBody.html) - Tells the object to be returned in the Response.

This allows us to return JSON.
{% gist ethankhall/4a6ae52d5348f56f6857 JobRequest.java %}

# XML files
## Web.xml
We still need a web.xml for the server to run properly. So here is what I have. It will take the InitalConfig class that we make earlier, and use that to start the system. It will then start a servlet up. 

{% gist ethankhall/4a6ae52d5348f56f6857 web.xml %}

## Logback.xml
For the logging we need a logback.xml.

{% gist ethankhall/4a6ae52d5348f56f6857 logback.xml %}

That is all you need to do to get this guy running. If you want to use a Gradle to run a Jetty server for testing.

{% highlight bash %}
./gradlew jettyRunWar
{% endhighlight %}

You can clone this entire project from [Github](https://github.com/ethankhall/mule). Please create issues that you find with the project on the [issues](https://github.com/ethankhall/mule/issues) page, you can also use it for requests.

# Further Reading

*  [@Import](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/context/annotation/Import.html) - Include other Spring Configuration classes
*  [@ImportResource](http://docs.spring.io/spring/docs/3.1.x/javadoc-api/org/springframework/context/annotation/ImportResource.html) - Include legacy XML files
