#### <span style="font-family: tahoma, arial, helvetica, sans-serif;">First clone down the AERPAW_DEV repo</span>

<pre><span style="font-family: tahoma, arial, helvetica, sans-serif;"><code class="language-bash">git clone https://github.com/aayamrajshakya/AERPAW_DEV.git&#10;</code></span></pre>

#### <span style="font-family: tahoma, arial, helvetica, sans-serif;">run before starting:</span>

<span style="font-family: tahoma, arial, helvetica, sans-serif;"><code>docker
network create</code>&nbsp;</span> \
<span style="font-family: tahoma, arial, helvetica, sans-serif;"><code>--driver=bridge</code></span>\
<span style="font-family: tahoma, arial, helvetica, sans-serif;"><code>--subnet=192.168.70.128/26</code></span>\
<span style="font-family: tahoma, arial, helvetica, sans-serif;"><code>-o "com.docker.network.bridge.name"="demo-oai"</code></span>\
<span style="font-family: tahoma, arial, helvetica, sans-serif;"><code>demo-oai-public-net</code></span>

#### <span style="font-family: tahoma, arial, helvetica, sans-serif;">Then simply cd into the cloned directory and build the image with docker tagging with an appropriate name.</span>

<pre><span style="font-family: tahoma, arial, helvetica, sans-serif;"><code class="language-bash">cd AERPAW_DEV&#10;sudo docker build . -t oaicoreandgnb&#10;&#10;</code></span></pre>

<span style="font-family: tahoma, arial, helvetica, sans-serif;">Use docker
images command to see the container id of this image</span>

#### <span style="font-family: tahoma, arial, helvetica, sans-serif;">sudo docker images</span>

<span style="font-family: tahoma, arial, helvetica, sans-serif;"><img src="../images/dockerimages.png" alt=""></span>

#### <span style="font-family: tahoma, arial, helvetica, sans-serif;">Then run the image that was built:</span>

#### <span style="font-family: tahoma, arial, helvetica, sans-serif;"><code class="language-bash">sudo docker run -it --privileged --network &lt;network-id&gt; -v /dev/bus/usb:/dev/bus/usb --net none --cap-add=NET_ADMIN --cap-add=SYS_NICE &lt;conatiner-id&gt; /bin/bash<br></code></span>
