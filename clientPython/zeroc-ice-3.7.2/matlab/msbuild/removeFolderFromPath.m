%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function removeFolderFromPath(p)
  rmpath(p);
  savepath();
  exit(0);
end
