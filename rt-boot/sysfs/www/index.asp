<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>RT-BOOT Web恢复终端</title>
<link rel="stylesheet" href="/style.css" type="text/css" />
<script type="text/javascript" src="/nvbar.js"></script>
</head>

<body>
	<div id="container">
		<div id="bootloader_console">
			RT-BOOT Web恢复终端
		</div>
		<div id="nav">
			<ul id="navbar"></ul>
		</div>
		<div id="page">
	<table class="system_info">
	<tr class="g">
		<td class="th">CPU</td>
		<td><% soc_info %></td>
	</tr>
	<tr>
		<td class="th">Flash</td>
		<td><% flash_info %></td>
	</tr>
	<tr class="g">
		<td class="th">内存</td>
		<td><% ram_info %></td>
	</tr>
	<tr>
		<td class="th">以太网</td>
		<td><% eth_info %></td>
	</tr>
	<tr class="g">
		<td class="th">时钟频率</td>
		<td><% clock_info %></td>
	</tr>
	<tr class="ed">
		<td class="th">版本</td>
		<td><% version_info %></td>
	</tr>
	</table>
		</div>
		<div class="clearfloat"> </div>
	</div>
	<script type="text/javascript">
		create_nvbar("/index.asp");
	</script>
</body>
</html>
