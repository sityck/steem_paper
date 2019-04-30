更新记录（API总结整理）

1、4.30日更新tags_api中的get_discussions_by_created函数
该函数用于获取当前最新博客信息
输入参数：{"tag":"bob","limit":2}，tag为博客中的标签参数，
注意：当发表博客时未填写该标签，或者标签值与当前搜索的tag参数值不匹配，则无法检索到
填写示例：json_metadata: JSON.stringify({"tags":["bob"]})

输出参数：返回博客内容以及创建的时间戳（参数名为create）
