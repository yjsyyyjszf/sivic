%{
    Copyright © 2016-2017 The Regents of the University of California.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    •   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
    •   Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
    •   None of the names of any campus of the University of California, the name
        "The Regents of the University of California," or the names of any of its
        contributors may be used to endorse or promote products derived from this
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.
%}
function [psi, theta, phi] = obtainEulerAnglesFromNormals(normMatrix)

normals = normMatrix';

if (abs(normals(3,1)) ~= 1)

    theta1 = -1 * asin( normals(3,1)); 
    theta2 = pi - theta1; 
    cosTheta1 = cos( theta1 ); 
    cosTheta2 = cos( theta2 ); 

    psi1   = atan2( ( normals(3,2)/cosTheta1 ) , ( normals(3,3)/cosTheta1 ) ); 
    psi2   = atan2( ( normals(3,2)/cosTheta2 ) , ( normals(3,3)/cosTheta2 ) ); 
    phi1   = atan2( ( normals(2,1)/cosTheta1 ) , ( normals(1,1)/cosTheta1 ) ); 
    phi2   = atan2( ( normals(2,1)/cosTheta2 ) , ( normals(1,1)/cosTheta2 ) ); 

    psi = psi1; 
    theta = theta1; 
    phi = phi1; 

else
    phi = pi; 
    if ( normals(3,1) == -1 )
        theta = pi/2; 
        psi = phi + atan2( normals(1,2) , normals(1,3) ); 
    else
        theta = -1 * pi/2; 
        psi = -1 * phi + atan2( (-1 * normals(1,2)) , (-1 * normals(1,3)) ); 
    end

end
