import discord, asyncio, os
from discord.ext import commands

game = discord.Game("Weather Broadcasting")
bot = commands.Bot(command_prefix = '!', status = discord.Status.online, activity = game)

@bot.command(aliases = ['안녕'])
async def hello(ctx):
    await ctx.send(f'hello!')

@bot.command(aliases = ['날씨'])
async def weather(ctx):
    f = open('C:/cansat_code/cansatData.txt', 'r', encoding = 'UTF-8')
    count = 0
    while 1:
        if f.readline() == '':
            break
        count += 1
    data = f.readlines()[count - 4 : count]
    embed = discord.Embed(title = "현재 날씨 정보", description = '', color = 0x4432a8)
    embed.add_field(name = "시간 및 기상상황", value = data[1], inline = False)
    await ctx.send(count, embed = embed)

@bot.command(aliases = ['영상'])
async def cam(ctx):
    embed = discord.Embed(title = "카메라 웹 스트리밍", url = "https://192.0.0.1", color = 0x4432a8)
    await ctx.send(embed = embed)

bot.run('ENTER_THE_TOKEN_OF_YOUR_BOT')