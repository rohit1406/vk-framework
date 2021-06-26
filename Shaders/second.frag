#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;    // Color output from subpass 1
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;    // Depth output from subpass 1

layout(location = 0) out vec4 color;

void main(){
    // color = subpassLoad(inputColor).rgba;
    // color.g = 0.0;
    int xHalf = 1366/2;
    if(gl_FragCoord.x > xHalf){
        float lowerBound = 0.99;
        float upperBound = 1;
        
        float depth = subpassLoad(inputDepth).r;
        
        // Scale the depth value between upperBound and lowerBound
        float depthColorScale = 1.0 - ((depth - lowerBound) / (upperBound - lowerBound));
        
        //color = vec4(depthColorScale, 0.0, 0.0, 1.0);
        color = vec4(subpassLoad(inputColor).rgb * depthColorScale, 1.0);
    }else{
        color = subpassLoad(inputColor).rgba;
    }
}
