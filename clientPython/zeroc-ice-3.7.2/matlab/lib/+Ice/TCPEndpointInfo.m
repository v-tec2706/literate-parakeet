classdef TCPEndpointInfo < Ice.IPEndpointInfo
    % TCPEndpointInfo   Summary of TCPEndpointInfo
    %
    % Provides access to a TCP endpoint information.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = TCPEndpointInfo(type, secure, underlying, timeout, compress, host, port, sourceAddress)
            if nargin == 0
                underlying = [];
                timeout = 0;
                compress = false;
                host = '';
                port = 0;
                sourceAddress = '';
            end
            obj = obj@Ice.IPEndpointInfo(type, false, secure, underlying, timeout, compress, host, port, sourceAddress);
        end
    end
end
