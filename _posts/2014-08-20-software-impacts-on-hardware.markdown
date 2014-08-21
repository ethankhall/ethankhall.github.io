---
layout: post
title:  "Hardware impications of Software"
date:   2014-08-20 20:26:13
categories: hardware
---

As software engineers we want to make code run faster. It's often only an issue when it's an issue in production.

When we write software we look for things that will make it run slow like:
- Create objects in loops
- Recomputing values multiple times
- Database cache misses

Something that we don't think about is hardware implications of code. Look at the following methods, what would you expect the runtime of the two to be (as a constant compared to the other).

{% highlight c %}
void loopOverSecondIndex() {
    for(int i = 1; i < SIZE; i++){
        for(int j = 1; j < SIZE; j++) {
            largeArray[i][j] += largeArray[i][j-1];
        }
    }
}

void loopOverFirstIndex() {
    for(int j = 1; j < SIZE; j++) {
        for(int i = 1; i < SIZE; i++){
            largeArray[i][j] += largeArray[i][j-1];
        }
    }
}
{% endhighlight %}

I would say the ratio is 1:1 from a quick glance. However running the following program we got some supprising results.

{% highlight c %}
{% include software/hardware-runtime/src/main/c/main.c %}
{% endhighlight %}

To run it we run (from \_include/software/hardware-runtime):

{% highlight bash %}
$> gradle mainExecutable && ./build/binaries/mainExecutable/main                          
:mainCExtractHeaders UP-TO-DATE
:compileMainExecutableMainC
:linkMainExecutable
:mainExecutable

BUILD SUCCESSFUL

Total time: 0.671 secs
First Index [i+1][j]
---- Stats for run ----
Max Runtime:        038741
Average Runtime:    032946
Min Runtime:        031661

Second Index [i][j+1]
---- Stats for run ----
Max Runtime:        013336
Average Runtime:    009877
Min Runtime:        009419
{% endhighlight %}

The loopOverSecondIndex method is almost 3.3 times as fast as loopOverFirstIndex. Wow... Thats a supprising result.

#### Dig into the results

So we need to go over some definitions:
- Cache
    - A processor has several of these. Let's call them L1 and L2. To get data into L1 it must first be moved into L2. To get into L2 it has to be in RAM. The process in which memory moves from RAM into a cache is hardware driven. When there is a cache miss, the processor will move to the next level to pull in values.
    - Caches decrease runtime. Some documentation can be found on [Wiki](https://en.wikipedia.org/wiki/CPU_cache).
- [Locality](https://en.wikipedia.org/wiki/Locality_of_reference)

So what does this mean to us as engineers?

The faster example (`[i][j+1]`) uses Locality to improve performance. When you use a varable like `foo[0][0]`, that isn't the only data that gets pulled into the cache, the surrounding memory comes with it. For example you will have `foo[0][0]` through `foo[0][3]` in the cache. In this example ever 4 requests will only have 1 cache miss.

In the slower example (`[i+1][j]`) you pull in `foo[0][0]` though `foo[0][3]` into the cache. Instead of the next request being `foo[0][1]` it's `foo[1][0]`. This means that there will be a cache miss EVERY time. Forcing you to go out to L2 or RAM to get the value.

Something that processor also do for you, is try to pre-fetch values from memory in an attempt to speed up the runtime. 

#### Sooo.... What does this mean?

If you care about runtime, you should thing about how things are layed out in memory and HOW you access things. If you have to loop over the column vs a row, you may want to transform the data to be where you can run along the row.
