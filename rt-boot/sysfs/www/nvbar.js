// JavaScript Document
function create_nvbar(nav_page)
{
	var nav_str = "/index.asp|系统信息,/upgrade.html|固件更新,/backup.html|固件备份,/reboot.html|重启,/about.html|关于";
	var nav_list = nav_str.split(",");
	var navbar = document.getElementById("navbar");

	for (var i = 0; i < nav_list.length; i++) 
	{
		if (!nav_list[i].indexOf("|"))
			continue;

		var nav_val = nav_list[i].split("|");

		var li = document.createElement("li");
		if (nav_page == nav_val[0]) 
		{
			li.className = "current_nav";
			li.appendChild(document.createTextNode(nav_val[1]));
		} 
		else 
		{
			var a = document.createElement("a");
			a.appendChild(document.createTextNode(nav_val[1]));
			a.setAttribute("href", nav_val[0]);
			li.appendChild(a);
		}

		navbar.appendChild(li);
	}
}