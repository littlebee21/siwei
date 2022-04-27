# !bin/bash/

git init

git clone https://github.com/littlebee21/siwei.git

#add file to push
git checkout -b

echo "hello" >> README.md

git add README.md
git commit -m "first commit"

#set branch and respos
git branch -M main

# 设置远程仓库，ssh方式的不需要使用密码
# git remote add origin https://github.com/littlebee21/siwei.git
git remote set-url origin git@github.com/littlebee21/siwei.git


#orgin 是名称 main是分治
git push -u origin main

