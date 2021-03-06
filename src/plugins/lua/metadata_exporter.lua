--[[
Copyright (c) 2016, Andrew Lewis <nerf@judo.za.org>
Copyright (c) 2016, Vsevolod Stakhov <vsevolod@highsecure.ru>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]--

-- A plugin that pushes metadata (or whole messages) to external services

local rspamd_http
local rspamd_logger = require "rspamd_logger"

local settings = {
  format = function(task)
    return task:get_content()
  end,
  mime_type = 'text/plain',
}

local opts = rspamd_config:get_all_opt('metadata_exporter')
if not opts then return end
local redis_params
local channel = opts['channel']
local url = opts['url']
if not (url or channel) then
  rspamd_logger.errx('No backends configured')
end
if channel then
  redis_params = rspamd_parse_redis_server('metadata_exporter')
  if not redis_params then
    rspamd_logger.errx(rspamd_config, 'No redis servers are specified')
    return
  end
end
if url then
  rspamd_http = require "rspamd_http"
end
if opts['select'] then
  settings.select = assert(loadstring(opts['select']))()
end
if opts['format'] then
  settings.format = assert(loadstring(opts['format']))()
end
if opts['mime_type'] then
  settings['mime_type'] = opts['mime_type']
end

local function metadata_exporter(task)
  local ret,conn,upstream
  local function http_callback(err, code, body, headers)
    if err then
      rspamd_logger.errx(task, 'got error %s in http callback', err)
    end
    if code ~= 200 then
      rspamd_logger.errx(task, 'got unexpected http status: %s', code)
    end
  end
  local function redis_set_cb(err, data)
    if err then
      rspamd_logger.errx(task, 'got error %s when publishing record on server %s',
          err, upstream:get_addr())
      upstream:fail()
    else
      upstream:ok()
    end
  end
  if settings.select then
    if not settings.select(task) then return end
    rspamd_logger.debugx(task, 'Message selected for processing')
  end
  local data = settings.format(task)
  if not data then
    rspamd_logger.debugx(task, 'Format returned non-truthy value: %1', data)
    return
  end
  if channel then
    ret,conn,upstream = rspamd_redis_make_request(task,
      redis_params, -- connect params
      nil, -- hash key
      true, -- is write
      redis_set_cb, --callback
      'PUBLISH', -- command
      {channel, data} -- arguments
    )
  end
  if url then
    rspamd_http.request({
      task=task,
      url=url,
      body=data,
      callback=http_callback,
      mime_type=settings['mime_type'],
    })
  end
end

rspamd_config:register_symbol({
  name = 'EXPORT_METADATA',
  type = 'postfilter',
  callback = metadata_exporter,
  priority = 10
})
